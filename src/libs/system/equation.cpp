/* Equations (ident = expr)
   Copyright (C) 2009--2012 Jarryd Beck

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
#include <tl/types/demand.hpp>
#include <tl/types/function.hpp>
#include <tl/types/range.hpp>
#include <tl/types/tuple.hpp>
#include <tl/types_util.hpp>
#include <tl/utility.hpp>

#include <vector>

#define XSTRING(x) STRING(x)
#define STRING(x) #x

namespace TransLucid
{

namespace
{
  //just to instantiate something for now
  bool bool_guard_hack = false;

  //-1 represents no end time
  constexpr int END_TIME_INFINITE = -1;

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
, m_endTime(END_TIME_INFINITE)
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
: m_name(name), m_system(system), m_bestfit(this, system)
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
  m_bestfit.setName(U"variable: " + m_name);
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
, m_compiled(other.m_compiled)
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
: m_guard(g), m_boolean(b), m_compiled(false), m_onlyConst(false),
  m_system(nullptr), m_priority(0)
{
}

void
GuardWS::compile() const
{
  //everything goes into nonconst right now
  if (m_guard == nullptr)
  {
    m_compiled = true;
    return;
  }

  Context k;

  //set time to zero for now
  k.perturb(DIM_TIME, Types::Intmp::create(0));

  //some trickery to evaluate guards at compile time
  Workshops::TupleWS* t = dynamic_cast<Workshops::TupleWS*>(m_guard.get());

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
      #if 0
      m_dimNonNon.insert(std::make_pair(val.first, val.second));
      #endif
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

    m_compiled = true;
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

    m_compiled = rhs.m_compiled;
    m_onlyConst = rhs.m_onlyConst;

    m_priority = rhs.m_priority;
    m_system = rhs.m_system;
  }

  return *this;
}

GuardWS makeGuardWithTime(const mpz_class& start)
{
  GuardWS g;
  g.addDimension(DIM_TIME,
    Types::Range::create(Range(&start, nullptr)));
  return g;
}

Constant
VariableWS::operator()(Context& k)
{
  //sort out an undefined reference for now
  if (bool_guard_hack)
  {
    //GuardWS g;
    //g.evaluate(k);
  }

  return m_bestfit(k);
}

Constant
VariableWS::operator()(Context& kappa, Context& delta)
{
  return m_bestfit(kappa, delta);
}

#if 0
//how to bestfit with a cache
//  until we find a priority that has valid equations and there are no demands
//  for dimensions, do:
//1. evaluate all the guards that are applicable to the current time
//2. if there are any demands for dimensions then return
//3. evaluate the booleans and guards for applicability
//4. if any of that requires dimensions then return
//5. then bestfit
Constant
VariableWS::operator()(Context& kappa, Context& delta)
{
  applicable_list applicable;
  applicable_list potential;
  applicable.reserve(m_equations.size());
  std::vector<dimension_index> demands;

  const mpz_class& theTime = Types::Intmp::get(kappa.lookup(DIM_TIME));

  //find all the applicable ones

  //for each priority...
  //if nothing was found at this priority then look at the next one
  for (auto priorityIter = m_priorityVars.rbegin();
       priorityIter != m_priorityVars.rend() && applicable.empty();
       ++priorityIter
  )
  {
    //make sure there are no potentials from the previous priority
    potential.clear();

    //look at everything created before this time
    const auto& thisPriority = priorityIter->second;
    for (auto provenanceIter = thisPriority.begin();
         provenanceIter != thisPriority.end() 
           && provenanceIter->first <= theTime;
         ++provenanceIter
    )
    {
      const auto& eqn_i = provenanceIter->second;
      int endTime = eqn_i->second.endTime();

      //if it is valid in this instant
      if (endTime == END_TIME_INFINITE || endTime > theTime)
      {
        //if it has a non-empty context guard
        if (eqn_i->second.validContext())
        {
          const GuardWS& guard = eqn_i->second.validContext();
          auto result = guard.evaluate(kappa, delta);

          if (result.first)
          {
            Tuple evalContext = result.second;
            std::copy(guard.demands().begin(), guard.demands().end(),
              std::back_inserter(demands));

            potential.push_back(ApplicableTuple(evalContext, eqn_i));
          }
        }
        else
        {
          potential.push_back(ApplicableTuple(Tuple(), eqn_i));
        }
      }
    }

    if (demands.size() > 0)
    {
      return Types::Demand::create(demands);
    }

    //go through the potential equations and check their applicability
    for (const auto& p : potential)
    {
      const auto& context = std::get<0>(p);
      if (context.begin() == context.end())
      {
        if (std::get<1>(p)->second.provenance() <= theTime)
        {
          applicable.push_back(p);
        }
      }
      else
      {
        //check that all the dimensions in context are in delta
        bool hasdemands = false;
        for (const auto& index : context)
        {
          if (!delta.has_entry(index.first))
          {
            demands.push_back(index.first);
            hasdemands = true;
          }
        }

        //don't bother doing this if there are dimensions not available
        //booleanTrue returns false if there are demands
        if (!hasdemands && tupleApplicable(context, kappa)
          && booleanTrue(std::get<1>(p)->second.validContext(), kappa, delta,
               demands)
        )
        {
          applicable.push_back(p);
        }
      }
    }

    //stop if looking at the tuples generated demands
    if (demands.size() > 0)
    {
      return Types::Demand::create(demands);
    }
  }

  //now we have something valid and no demands are needed
  return bestfit(applicable, kappa, delta);
}

Constant
VariableWS::operator()(Context& k)
{

  //std::cerr << "evaluating variable " << m_name 
  //<< ", context: " 
  //<< std::endl;
  //k.print(std::cerr);
  //std::cerr << std::endl;

  applicable_list applicable;
  applicable.reserve(m_equations.size());

  const mpz_class& theTime = Types::Intmp::get(k.lookup(DIM_TIME));

  //find all the applicable ones

  //for each priority...
  //if nothing was found at this priority then look at the next one
  for (auto priorityIter = m_priorityVars.rbegin();
       priorityIter != m_priorityVars.rend() && applicable.empty();
       ++priorityIter
  )
  {
    //look at everything created before this time
    const auto& thisPriority = priorityIter->second;
    for (auto provenanceIter = thisPriority.begin();
         provenanceIter != thisPriority.end() 
           && provenanceIter->first <= theTime;
         ++provenanceIter
    )
    {
      const auto& eqn_i = provenanceIter->second;
      int endTime = eqn_i->second.endTime();

      if (endTime == END_TIME_INFINITE || endTime > theTime)
      {
        if (eqn_i->second.validContext())
        {
          const GuardWS& guard = eqn_i->second.validContext();
          auto result = guard.evaluate(k);

          if (result.first && tupleApplicable(result.second, k)
            && booleanTrue(guard, k)
          )
          {
            applicable.push_back
              (ApplicableTuple(result.second, eqn_i));
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
  }

  return bestfit(applicable, k);
}

template <typename... Delta>
Constant
VariableWS::bestfit(const applicable_list& applicable, Context& k, 
  Delta&&... delta)
{

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
    return (*std::get<1>(applicable.front())->second.equation())(k, delta...);
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
      return (*std::get<1>(*newestBest.front())->second.equation())
        (k, delta...);
    }
    else
    {
      //std::cerr << m_name << ": multiple newest" << std::endl;

      //evaluate everything first
      std::vector<Constant> bestEvaluated;
      for (const auto& best : newestBest)
      {
        //best is an applicable_list::const_iterator
        bestEvaluated.push_back((*std::get<1>(*best)->second.equation())
          (k, delta...));
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
#endif

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

void
VariableWS::addEquation(uuid id, Parser::Variable eqn, int time)
{
  m_bestfit.addEquation(id, eqn, time);
}

void
VariableWS::addEquation(uuid id, Parser::RawInput input, int time)
{
  m_bestfit.addEquation(id, input, time);
}

bool
VariableWS::del(uuid id, size_t time)
{
  return m_bestfit.del(id, time);
}

bool
VariableWS::repl(uuid id, size_t time, Parser::Line line)
{
  return m_bestfit.repl(id, time, line);
}

Tree::Expr
VariableWS::group(const std::list<EquationDefinition>& lines)
{
  //all the definitions in lines are guaranteed to have a parsed
  //definition

  //create a conditional best fitter of all these equations
  Tree::ConditionalBestfitExpr best;
  for (auto& l : lines)
  {
    auto eqn = get<Parser::Variable>(l.parsed().get());
    //auto eqn = get<Parser::Equation>(l.parsed().get());

    if (eqn == nullptr)
    {
      throw "internal compiler error: " __FILE__ ":" XSTRING(__LINE__);
    }

    best.declarations.push_back(std::make_tuple
    (
      l.start(),
      std::get<1>(eqn->eqn),
      std::get<2>(eqn->eqn),
      std::get<3>(eqn->eqn)
    ));

#if 0
    best.declarations.push_back(std::make_tuple
    (
      l.start(),
      std::get<1>(*eqn),
      std::get<2>(*eqn),
      std::get<3>(*eqn)
    ));
#endif
  }

  return best;
}

void
EquationWS::del(size_t time)
{
  m_endTime = time;
}

//template
//std::pair<bool, Tuple>
//GuardWS::evaluate(Context& k) const;

}
