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
#include <tl/types/range.hpp>
#include <tl/types/tuple.hpp>
#include <tl/utility.hpp>

#include <boost/uuid/uuid_generators.hpp>

#include <vector>

namespace TransLucid
{

namespace
{
boost::uuids::basic_random_generator<boost::mt19937> uuid_generator;
}

EquationWS::EquationWS(const u32string& name, const GuardWS& valid, WS* h)
: m_name(name), m_validContext(valid), m_h(h),
  m_id(uuid_generator())
{
}

EquationWS::EquationWS()
: m_h(0), m_id(boost::uuids::nil_generator()())
{
}

EquationWS::~EquationWS()
{
}

VariableWS::VariableWS(const u32string& name)
: m_name(name)
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
, m_tupleConstDims(other.m_tupleConstDims)
, m_dimConstConst(other.m_dimConstConst)
, m_dimConstNon(other.m_dimConstNon)
, m_dimNonConst(other.m_dimNonConst)
, m_dimNonNon(other.m_dimNonNon)
, m_onlyConst(other.m_onlyConst)
, m_timeStart(0)
, m_timeEnd(0)
, m_system(other.m_system)
{
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
}

GuardWS::GuardWS(WS* g, WS* b)
: m_guard(g), m_boolean(b), m_onlyConst(false),
  m_timeStart(0), m_timeEnd(0), m_system(nullptr)
{
  if (b != 0)
  {
    std::cerr << "I don't know how to handle boolean guards" << std::endl;
    throw "I don't know how to handle boolean guards";
  }

  if (g == 0)
  {
    return;
  }

  Context k;

  //set time to zero for now
  k.perturb(DIM_TIME, Types::Intmp::create(0));

  //some trickery to evaluate guards at compile time
  Workshops::TupleWS* t = dynamic_cast<Workshops::TupleWS*>(g);

  //For now guards must be a literal tuple
  if (t != 0)
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
    m_tupleConstDims = m_dimConstConst;
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
    delete m_timeStart;
    delete m_timeEnd;
    m_timeStart = 0;
    m_timeEnd = 0;

    m_guard = rhs.m_guard;
    m_boolean = rhs.m_boolean;

    m_tupleConstDims = rhs.m_tupleConstDims;

    m_dimConstConst = rhs.m_dimConstConst;
    m_dimConstNon = rhs.m_dimConstNon;
    m_dimNonConst = rhs.m_dimNonConst;
    m_dimNonNon = rhs.m_dimNonNon;

    m_onlyConst = rhs.m_onlyConst;

    try 
    {
      if (rhs.m_timeStart)
      {
        m_timeStart = new mpz_class(*rhs.m_timeStart);
      }

      if (rhs.m_timeEnd)
      {
        m_timeEnd = new mpz_class(*rhs.m_timeEnd);
      }
    }
    catch (...)
    {
      delete m_timeStart;
      delete m_timeEnd;
      m_timeStart = 0;
      m_timeEnd = 0;
      throw;
    }
  }

  return *this;
}

Tuple
GuardWS::evaluate(Context& k) const
{
  if (m_onlyConst)
  {
    return m_tupleConstDims;
  }

  tuple_t t = m_dimConstConst;

  if (m_guard)
  {
    //start with the const dimensions and evaluate the non-const ones
    //Constant v = (*m_guard)(k);
    //t.insert(m_dimConstConst.begin(), m_dimConstConst.end());

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

    //still need to remove this magic
    #if 0
    if (v.index() == TYPE_INDEX_TUPLE)
    {
      for(const Tuple::value_type& value : Types::Tuple::get(v))
      {
        if (t.find(value.first) != t.end())
        {
          throw InvalidGuard();
        }

        t.insert(value);
      }
    }
    else
    {
      throw ParseError(__FILE__ ":" STRING_(__LINE__)
                       U": guard is not a tuple");
    }
    #endif
  }

  //add the time
  tuple_t::const_iterator timeIter = t.find(DIM_TIME);
  if (timeIter != t.end())
  {
    //std::cerr << "warning: user specified time, don't know what to do" 
    //  << std::endl;
    //fix this, don't know what to do if the user has specified time
    
    // This is complicated, at t, we can't add an equation with time t - i,
    // i > 0, effectively changing the past. However, we can add equations
    // for time > t.
  }
  else
  {
    t[DIM_TIME] = 
      Types::Range::create(Range(m_timeStart, m_timeEnd));
  }

  return Tuple(t);
  //TaggedConstant v = (*m_guard)(k);
}

GuardWS makeGuardWithTime(const mpz_class& start)
{
  GuardWS g;
  g.addDimension(DIM_TIME,
    Types::Range::create(Range(&start, 0)));
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

  #if 0
  std::cerr << "evaluating variable "
            << m_name << ", context: " << std::endl;
  k.print(std::cerr);
  std::cerr << std::endl;
  #endif

  typedef std::tuple<Tuple, UUIDEquationMap::const_iterator> ApplicableTuple;
  typedef std::vector<ApplicableTuple> applicable_list;
  applicable_list applicable;
  applicable.reserve(m_equations.size());

  //find all the applicable ones
  for (UUIDEquationMap::const_iterator eqn_i = m_equations.begin();
      eqn_i != m_equations.end(); ++eqn_i)
  {
    if (eqn_i->second.validContext())
    {
      try
      {
        const GuardWS& guard = eqn_i->second.validContext();
        Tuple evalContext = guard.evaluate(k);
        if (tupleApplicable(evalContext, k) 
          //we don't know how to do booleans for now
          //&& booleanTrue(guard, k)
        )
        {
          applicable.push_back
            (ApplicableTuple(evalContext, eqn_i));
         }
      }
      catch (InvalidGuard& e)
      {
      }
    }
    else
    {
      applicable.push_back
        (ApplicableTuple(Tuple(), eqn_i));
    }
    #if 0
    if (equationValid(eqn_i->second, k))
    {
    }
    #endif
  }

  //std::cout << "have " << applicable.size() << " applicable equations" << std::endl;
  if (applicable.size() == 0)
  {
    std::cerr << "undef for " << m_name << std::endl;
    return Types::Special::create(SP_UNDEF);
  }
  else if (applicable.size() == 1)
  {
    //std::cerr << "running equation " << std::get<1>(applicable.front())->id()
    //<< std::endl;
    return (*std::get<1>(applicable.front())->second.equation())(k);
  }

  //find the best ones
  std::vector<applicable_list::const_iterator> bestIters;

  for (applicable_list::const_iterator i = applicable.begin();
       i != applicable.end(); ++i)
  {
    bool best = true;
    for (applicable_list::const_iterator j = applicable.begin();
         j != applicable.end(); ++j)
    {

      if (i != j && !tupleRefines(std::get<0>(*i), std::get<0>(*j)))
      {
        best = false;
      }

    #if 0
    if (bestIter == applicable.end())
    {
      bestIter = iter;
    }
    else if (tupleRefines(std::get<0>(*iter), std::get<0>(*bestIter)))
    {
      bestIter = iter;
    }
    else if (!tupleRefines(std::get<0>(*bestIter), std::get<0>(*iter)))
    {
      bestIter = applicable.end();
    }
    #endif

    }

    if (best)
    {
      bestIters.push_back(i);
    }
  }
 
  //std::cerr << "running equation " << std::get<1>(*bestIter)->id()
  //<< std::endl;

  if (bestIters.size() == 1)
  {
    return (*std::get<1>(*bestIters.front())->second.equation())(k);
  }
  else
  {
    return Types::Special::create(SP_MULTIDEF);
  }
}

uuid
VariableWS::addEquation(EquationWS* e, size_t time)
{
  return m_equations.insert(std::make_pair(e->id(), *e)).first->first;
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
  guard.setTimeStart(time);

  EquationWS eq(name, guard, e);

  return m_equations.insert(std::make_pair(eq.id(), eq)).first->first;
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
  m_validContext.setTimeEnd(time);
}

}
