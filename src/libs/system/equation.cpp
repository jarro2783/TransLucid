/* Equations (ident = expr)
   Copyright (C) 2009, 2010 Jarryd Beck and John Plaice

This file is part of TransLucid.

TransLucid is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 3, or (at your option)
any later version.

TransLucid is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with TransLucid; see the file COPYING.  If not see
<http://www.gnu.org/licenses/>.  */

/** @file equation.cpp
 * All of the code related to equations. The main part of bestfitting
 * and the management of equations is done here.
 */

#include <tl/equation.hpp>
#include <tl/eval_workshops.hpp>
#include <tl/output.hpp>
#include <tl/range.hpp>
#include <tl/types/function.hpp>
#include <tl/types/range.hpp>
#include <tl/types/tuple.hpp>
#include <tl/types_util.hpp>
#include <tl/utility.hpp>

#include <vector>

namespace TransLucid
{

namespace
{
  template <typename Container>
  Constant
  evaluateBestselect(Context& k, Container&& results, const Constant& fn)
  {
    Constant currentVal = results.front();

    auto iter = results.begin();
    ++iter;

    while (iter != results.end())
    {
      Constant fn2 = applyFunction(k, fn, currentVal);
      currentVal = applyFunction(k, fn2, *iter);
      ++iter;
    }

    return currentVal;
  }
}

EquationWS::EquationWS(const u32string& name, const GuardWS& valid, WS* h,
  int provenance)
: m_name(name), m_validContext(valid), m_h(h)
, m_id(generate_uuid()), m_provenance(provenance)
, m_priority(valid.priority())
{
}

EquationWS::EquationWS()
: m_h(0), m_id(generate_nil_uuid()), m_provenance(0), m_priority(0)
{
}

EquationWS::~EquationWS()
{
}

VariableWS::VariableWS(const u32string& name, System& system)
: m_name(name), m_system(system)
#if 0
 ,m_compileBestFit
  (
    new CompileBestFit
    (
      m_equations,
      m_variables
    )
  )
#endif
{
}

VariableWS::~VariableWS()
{
  //cleanup best fit variables
}

GuardWS::GuardWS(const GuardWS& other)
: m_guard(other.m_guard)
, m_boolean(other.m_boolean)
, m_dimConstConst(other.m_dimConstConst)
, m_dimConstNon(other.m_dimConstNon)
, m_dimNonConst(other.m_dimNonConst)
, m_dimNonNon(other.m_dimNonNon)
, m_onlyConst(other.m_onlyConst)
, m_system(other.m_system)
, m_priority(other.m_priority)
{
#if 0
  try
  {
    if (other.m_timeStart) 
    {
      m_timeStart = new mpz_class(*other.m_timeStart);
    }

    if (other.m_timeEnd)
    {
      m_timeEnd = new mpz_class(*other.m_timeEnd);
    }
  }
  catch (...)
  {
    delete m_timeStart;
    delete m_timeEnd;
    throw;
  }
#endif
}

GuardWS::GuardWS(WS* g, WS* b)
: m_guard(g), m_boolean(b), m_onlyConst(false),
  m_system(nullptr), m_priority(0)
{
  if (g == nullptr)
  {
    return;
  }

  Context k;

  //set time to zero for now
  k.perturb(DIM_TIME, Types::Intmp::create(0));

  //some trickery to evaluate guards at compile time
  Workshops::TupleWS* t = dynamic_cast<Workshops::TupleWS*>(g);

  //For now guards must be a literal tuple
  if (t != nullptr)
  {
    const auto& pairs = t->getElements();
    System& s = t->getSystem();
    m_system = &s;

    for (const auto& val : pairs)
    {
      bool lhsConst = true;
      bool rhsConst = true;
      Constant lhs = val.first->operator()(k);

      if (lhs.index() == TYPE_INDEX_SPECIAL)
      {
        lhsConst = false;
      }

      dimension_index dimIndex = 
        lhs.index() == TYPE_INDEX_DIMENSION 
        ? get_constant<dimension_index>(lhs)
        : s.getDimensionIndex(lhs);

      Constant rhs = val.second->operator()(k);

      if (rhs.index() == TYPE_INDEX_SPECIAL)
      {
        rhsConst = false;
      }

      //we'll work out whether both left-hand sides and right-hand sides
      //are constant, and stick them in a container to evaluate the
      //remainder later
      if (lhsConst)
      {
        if (rhsConst)
        {
          m_dimConstConst.insert(std::make_pair(dimIndex, rhs));
        }
        else
        {
          m_dimConstNon.insert(std::make_pair(dimIndex, val.second));
        }
      }
      else
      {
        if (rhsConst)
        {
          m_dimNonConst.insert(std::make_pair(val.first, rhs));
        }
        else
        {
          m_dimNonNon.insert(std::make_pair(val.first, val.second));
        }
      }
    }

    if (m_dimConstNon.size() == 0 && m_dimNonConst.size() == 0
        && m_dimNonNon.size() == 0)
    {
      m_onlyConst = true;
    }

    //find a priority dimension setting
    auto priorityIter = m_dimConstConst.find(DIM_PRIORITY);
    if (priorityIter != m_dimConstConst.end())
    {
      if (priorityIter->second.index() == TYPE_INDEX_INTMP)
      {
        m_priority = Types::Intmp::get(priorityIter->second).get_si();
        m_dimConstConst.erase(priorityIter);
      }
    }
  }
  else
  {
    std::cerr << "guard is not a tuple" << std::endl;
    throw "guard is not a tuple";
  }
}

GuardWS& 
GuardWS::operator=(const GuardWS& rhs)
{
  if (this != &rhs)
  {
    m_guard = rhs.m_guard;
    m_boolean = rhs.m_boolean;

    m_dimConstConst = rhs.m_dimConstConst;
    m_dimConstNon = rhs.m_dimConstNon;
    m_dimNonConst = rhs.m_dimNonConst;
    m_dimNonNon = rhs.m_dimNonNon;

    m_onlyConst = rhs.m_onlyConst;

    m_priority = rhs.m_priority;
    m_system = rhs.m_system;
  }

  return *this;
}

Tuple
GuardWS::evaluate(Context& k) const
{
  tuple_t t = m_dimConstConst;

  if (m_guard)
  {
    //start with the const dimensions and evaluate the non-const ones

    //evaluate the ones left
    for (const auto& constNon : m_dimConstNon)
    {
      Constant ord = constNon.second->operator()(k);
      t.insert(std::make_pair(constNon.first, ord));
    }

    for (const auto& nonConst : m_dimNonConst)
    {
      Constant dim = nonConst.first->operator()(k);

      dimension_index index =
        dim.index() == TYPE_INDEX_DIMENSION 
        ? get_constant<dimension_index>(dim)
        : m_system->getDimensionIndex(dim);

      t.insert(std::make_pair(index, nonConst.second));
    }

    for (const auto& nonNon : m_dimNonNon)
    {
      Constant dim = nonNon.first->operator()(k);
      Constant ord = nonNon.second->operator()(k);

      dimension_index index =
        dim.index() == TYPE_INDEX_DIMENSION 
        ? get_constant<dimension_index>(dim)
        : m_system->getDimensionIndex(dim);

      t.insert(std::make_pair(index, ord));
    }
  }

  return Tuple(t);
}

GuardWS makeGuardWithTime(const mpz_class& start)
{
  GuardWS g;
  g.addDimension(DIM_TIME,
    Types::Range::create(Range(&start, nullptr)));
  return g;
}

#if 0
bool VariableWS::equationValid(const EquationWS& e, const Tuple& k)
{
}
#endif

Constant
VariableWS::operator()(Context& k)
{

  //std::cerr << "evaluating variable " << m_name 
  //<< ", context: " 
  //<< std::endl;
  //k.print(std::cerr);
  //std::cerr << std::endl;

  typedef std::tuple<Tuple, UUIDEquationMap::const_iterator> ApplicableTuple;
  typedef std::vector<ApplicableTuple> applicable_list;
  applicable_list applicable;
  applicable.reserve(m_equations.size());

  const mpz_class& theTime = Types::Intmp::get(k.lookup(DIM_TIME));

  //find all the applicable ones
  for (auto priorityIter = m_priorityVars.rbegin();
       priorityIter != m_priorityVars.rend() && applicable.empty();
       ++priorityIter
  )
  {
    const auto& thisPriority = priorityIter->second;
    for (auto provenanceIter = thisPriority.begin();
         provenanceIter != thisPriority.end() 
           && provenanceIter->first <= theTime;
         ++provenanceIter
    )
    {
      const auto& eqn_i = provenanceIter->second;
      if (eqn_i->second.validContext())
      {
        const GuardWS& guard = eqn_i->second.validContext();
        Tuple evalContext = guard.evaluate(k);

        if (tupleApplicable(evalContext, k)
          && booleanTrue(guard, k)
        )
        {
          applicable.push_back
            (ApplicableTuple(evalContext, eqn_i));
        }
      }
      else
      {
        if (eqn_i->second.provenance() <= theTime)
        {
          applicable.push_back
            (ApplicableTuple(Tuple(), eqn_i));
        }
      }
    }
  }

  //for whichever priority something was chosen, they will have been added from
  //oldest to newest, so we now have a list in order of oldest to newest

  //std::cout << "have " << applicable.size() << " applicable equations" 
  //          << std::endl;
  if (applicable.size() == 0)
  {
    //std::cerr << "undef for " << m_name << std::endl;
    return Types::Special::create(SP_UNDEF);
  }
  else if (applicable.size() == 1)
  {
    //std::cerr << "running equation " << std::get<1>(applicable.front())->id()
    //<< std::endl;
    return (*std::get<1>(applicable.front())->second.equation())(k);
  }

  //if there is more than applicable equation, find the best
  std::vector<applicable_list::const_iterator> bestIters;

  for (applicable_list::const_iterator i = applicable.begin();
       i != applicable.end(); ++i)
  {
    bool best = true;
    for (applicable_list::const_iterator j = applicable.begin();
         j != applicable.end(); ++j)
    {
      if (i != j && !tupleRefines(std::get<0>(*i), std::get<0>(*j), true))
      {
        best = false;
      }
    }

    if (best)
    {
      bestIters.push_back(i);
    }
  }

  //the list of best will be in order from oldest to newest, so go through 
  //from the back and find everything with the highest provenance
  if (bestIters.size() >= 1)
  {
    //std::cerr << m_name << ": " << bestIters.size() << " best" << std::endl;
    std::vector<applicable_list::const_iterator> newestBest;

    auto iter = bestIters.rbegin();
    int latest = std::get<1>(**iter)->second.provenance();
    while (iter != bestIters.rend() &&
           latest == std::get<1>(**iter)->second.provenance())
    {
      newestBest.push_back(*iter);
      ++iter;
    }

    if (newestBest.size() == 1)
    {
      //std::cerr << m_name << ": One newest" << std::endl;
      return (*std::get<1>(*newestBest.front())->second.equation())(k);
    }
    else
    {
      //std::cerr << m_name << ": multiple newest" << std::endl;

      //evaluate everything first
      std::vector<Constant> bestEvaluated;
      for (const auto& best : newestBest)
      {
        //best is an applicable_list::const_iterator
        bestEvaluated.push_back((*std::get<1>(*best)->second.equation())(k));
      }

      //find the bestselect
      auto bestselectName = U"bestselect_" + m_name;
      auto findIdent = m_system.lookupIdentifiers();
      WS* bestselect = findIdent.lookup(bestselectName);

      if (bestselect != nullptr)
      {
        Constant fn = (*bestselect)(k);
        return evaluateBestselect(k, bestEvaluated, fn);
      }
      else
      {
        //find default bestselect
        WS* bestselectDefault = findIdent.lookup(U"bestselect__");

        if (bestselectDefault == nullptr)
        {
          //we can blow up here
          throw "No default bestselect defined";
        }

        Constant fn = (*bestselectDefault)(k);
        return evaluateBestselect(k, bestEvaluated, fn);
      }
    }
  }
  else
  {
    //std::cerr << m_name << ": no best" << std::endl;
    return Types::Special::create(SP_UNDEF);
  }
 
  //std::cerr << "running equation " << std::get<1>(*bestIter)->id()
  //<< std::endl;

  if (bestIters.size() == 1)
  {
    return (*std::get<1>(*bestIters.front())->second.equation())(k);
  }
  else
  {
    //std::cerr << "multidef for " << m_name << std::endl;
    return Types::Special::create(SP_MULTIDEF);
  }
}

uuid
VariableWS::addEquation(EquationWS* e, size_t time)
{
  auto uiter = m_equations.insert(std::make_pair(e->id(), *e)).first;

  int priority = e->priority();

  //insert in the priority list
  auto iter = m_priorityVars.find(priority);

  if (iter == m_priorityVars.end())
  {
    iter = m_priorityVars.insert(std::make_pair(priority, ProvenanceList()))
      .first;
  }

  iter->second.push_back(std::make_pair(time, uiter));

  return uiter->first;
}

uuid
VariableWS::addEquation
(
  const u32string& name, 
  GuardWS guard, 
  WS* e, 
  size_t time
)
{
  //guard.setTimeStart(time);

  EquationWS eq(name, guard, e, time);

  return addEquation(&eq, time);

  //return m_equations.insert(std::make_pair(eq.id(), eq)).first->first;
}

bool
VariableWS::delexpr(uuid id, size_t time)
{
  UUIDEquationMap::iterator iter = m_equations.find(id);

  if (iter != m_equations.end())
  {
    iter->second.del(time);
  }
  return true;
}

bool
VariableWS::replexpr(uuid id, size_t time, const GuardWS& guard, WS* expr)
{
  UUIDEquationMap::iterator iter = m_equations.find(id);

  if (iter != m_equations.end())
  {
    //not sure how to do this yet, but how about we turn off that warning
    return true;
  }
  else
  {
    return false;
  }
}

void
EquationWS::del(size_t time)
{
  //m_validContext.setTimeEnd(time);
}

Constant
ConditionalBestfitWS::operator()(Context& k)
{
  return (*m_var)(k);
}

}
