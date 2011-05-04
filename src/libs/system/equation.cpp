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

#include <tl/equation.hpp>
#include <tl/range.hpp>
#include <tl/utility.hpp>

namespace TransLucid
{

boost::uuids::basic_random_generator<boost::mt19937>
EquationHD::m_generator;

VariableHD::VariableHD(const u32string& name, HD* system)
: m_name(name)
 ,m_system(system)
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

VariableHD::~VariableHD()
{
  //cleanup best fit variables
}

GuardHD::GuardHD(const GuardHD& other)
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

GuardHD& 
GuardHD::operator=(const GuardHD& rhs)
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
GuardHD::evaluate(const Tuple& k) const
{
  tuple_t t = m_dimensions;

  if (m_guard)
  {
    TaggedConstant v = (*m_guard)(k);

    //still need to remove this magic
    if (v.first.index() == TYPE_INDEX_TUPLE)
    {
      for(const Tuple::value_type& value : v.first.value<Tuple>())
      {
        if (t.find(value.first) != t.end())
        {
          throw InvalidGuard();
        }

        t.insert(std::make_pair(value.first, value.second));
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
    std::cerr << "warning: user specified time, don't know what to do" 
      << std::endl;
    //fix this, don't know what to do if the user has specified time
  }
  else
  {
    t[DIM_TIME] = 
      Constant(Range(m_timeStart, m_timeEnd),
               TYPE_INDEX_RANGE);
  }

  return Tuple(t);
  //TaggedConstant v = (*m_guard)(k);
}

GuardHD makeGuardWithTime(const mpz_class& start)
{
  GuardHD g;
  g.addDimension(DIM_TIME,
    Constant(Range(&start, 0), TYPE_INDEX_RANGE));
  return g;
}

bool VariableHD::equationValid(const EquationHD& e, const Tuple& k)
{
}

TaggedConstant
VariableHD::operator()(const Tuple& k)
{

  #if 0
  std::cerr << "evaluating variable "
            << m_name << ", context: " << std::endl;
  k.print(std::cerr);
  std::cerr << std::endl;
  #endif

  typedef std::tuple<Tuple, UUIDEquationMap::const_iterator> ApplicableTuple;
  typedef std::list<ApplicableTuple> applicable_list;
  applicable_list applicable;

  //find all the applicable ones

  #if 0
  Tuple::const_iterator iditer = k.find(DIM_ID);

  if (iditer != k.end())
  {
    try
    {
      
      const u32string& id = iditer->second.value<String>().value();
      SplitID split(id);
      u32string begin = split.first();
      u32string end = split.last();

      //std::cerr << "looking for id: " <<
        //utf32_to_utf8(iditer->second.value<String>().value()) << std::endl;
      //VariableMap::const_iterator viter =
      //  m_variables.find(iditer->second.value<String>().value());
      VariableMap::const_iterator viter =
        m_variables.find(begin);
      //std::cout << "looking for "
      //          << iditer->second.value<String>().value() << std::endl;
      if (viter == m_variables.end())
      {
        //std::cerr << "not found" << std::endl;
        return TaggedConstant(Constant(Special(Special::UNDEF),
                              TYPE_INDEX_SPECIAL), k);
      }
      else
      {
        tuple_t kp = k.tuple();
        if (end.empty())
        {
          kp.erase(DIM_ID);
        }
        else
        {
          kp[DIM_ID] = Constant(String(end), TYPE_INDEX_USTRING);
        }
        return (*viter->second)(Tuple(kp));
      }
    }
    catch (std::bad_cast& e)
    {
      return TaggedConstant(Constant(Special(Special::DIMENSION),
                            TYPE_INDEX_SPECIAL), k);
    }
  }
  #endif

  for (UUIDEquationMap::const_iterator eqn_i = m_equations.begin();
      eqn_i != m_equations.end(); ++eqn_i)
  {
    if (eqn_i->second.validContext())
    {
      try
      {
        const GuardHD& guard = eqn_i->second.validContext();
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
    return TaggedConstant(Constant(Special(Special::UNDEF),
                          TYPE_INDEX_SPECIAL),k);
  }
  else if (applicable.size() == 1)
  {
    //std::cerr << "running equation " << std::get<1>(applicable.front())->id()
    //<< std::endl;
    return (*std::get<1>(applicable.front())->second.equation())(k);
  }

  //find the best ones
  std::list<applicable_list::const_iterator> bestIters;

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
    return TaggedConstant(Constant(Special(Special::MULTIDEF),
                          TYPE_INDEX_SPECIAL),k);
  }
}

uuid
VariableHD::addEquation(EquationHD* e, size_t time)
{
  return m_equations.insert(std::make_pair(e->id(), *e)).first->first;
}

uuid
VariableHD::addEquation
(
  const u32string& name, 
  GuardHD guard, 
  HD* e, 
  size_t time
)
{
  guard.setTimeStart(time);

  EquationHD eq(name, guard, e);

  return m_equations.insert(std::make_pair(eq.id(), eq)).first->first;
}

#if 0
std::pair<uuid, VariableHD::UUIDEquationMap::iterator>
VariableHD::addExprInternal(const Tuple& k, HD* e)
{
  Tuple::const_iterator iter = k.find(DIM_ID);
  if (iter == k.end())
  {
    return addExprActual(k, e);
  }
  else
  {
    const String* id = iter->second.valuep<String>();
    if (id == 0)
    {
      return std::make_pair(nil_uuid(), m_equations.end());
    }

    SplitID split(id->value());

    //add the equation, don't add any id dimension if the end is empty
    u32string begin = split.first();
    u32string end = split.last();

    tuple_t kp = k.tuple();
    if (end.size() != 0)
    {
      kp[DIM_ID] = Constant(String(end), TYPE_INDEX_USTRING);
    }
    else
    {
      kp.erase(DIM_ID);
    }

    return addToVariableActual(begin, Tuple(kp), e);
  }
}
#endif

bool
VariableHD::delexpr(uuid id, size_t time)
{
  UUIDEquationMap::iterator iter = m_equations.find(id);

  if (iter != m_equations.end())
  {
    iter->second.del(time);
  }
  return true;
}

bool
VariableHD::replexpr(uuid id, size_t time, const GuardHD& guard, HD* expr)
{
}

void
EquationHD::del(size_t time)
{
  m_validContext.setTimeEnd(time);
}

}
