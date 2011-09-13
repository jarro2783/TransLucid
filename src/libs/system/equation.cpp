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
, m_dimensions(other.m_dimensions)
, m_timeStart(0)
, m_timeEnd(0)
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

    try 
    {
      m_dimensions = rhs.m_dimensions;
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
  tuple_t t = m_dimensions;

  if (m_guard)
  {
    TaggedConstant v = (*m_guard)(k);

    //still need to remove this magic
    if (v.first.index() == TYPE_INDEX_TUPLE)
    {
      for(const Tuple::value_type& value : Types::Tuple::get(v.first))
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

TaggedConstant
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
        if (tupleApplicable(evalContext, k) && booleanTrue(guard, k))
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
    return TaggedConstant(Types::Special::create(SP_UNDEF), k);
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
    return TaggedConstant(Types::Special::create(SP_MULTIDEF), k);
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
