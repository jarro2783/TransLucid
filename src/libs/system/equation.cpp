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
      BOOST_FOREACH(const Tuple::value_type& value, v.first.value<Tuple>())
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

  Tuple(t).print(std::cerr);
  std::cerr << std::endl;

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

inline std::pair<uuid, VariableHD::Equations::iterator>
VariableHD::addExprActual(const Tuple& k, HD* h)
{
  std::cerr << "defining an equation" << std::endl;
  GuardHD g;
  Tuple::const_iterator giter = k.find(DIM_VALID_GUARD);
  if (giter != k.end())
  {
    g = giter->second.value<Guard>().value();
  }

  Tuple::const_iterator titer = k.find(DIM_TIME);
  if (titer != k.end())
  {
    std::cerr << "adding time to equation definition" << std::endl;
    g.addDimension(DIM_TIME, 
      Constant(Range(&titer->second.value<Intmp>().value(), 0), 
               TYPE_INDEX_RANGE));
  }

  auto adder =
    [this] (const EquationHD& e) -> std::pair<uuid, Equations::iterator>
  {
    auto& id = e.id();
    return std::make_pair(
      id,
      this->m_equations.insert(std::make_pair(id, e)).first
    );
  };

  //if (g)
  //{
    return adder(EquationHD(m_name, g, h));
  //}
  //else
  //{
  //  return adder(EquationHD(m_name, 
  //    makeGuardWithTime(titer->second.value<Intmp>.value()), h));
  //}
}

bool VariableHD::equationValid(const EquationHD& e, const Tuple& k)
{
}

TaggedConstant
VariableHD::operator()(const Tuple& k)
{

  std::cerr << "evaluating variable "
            << m_name << ", context: " << std::endl;
  k.print(std::cerr);
  std::cerr << std::endl;

  typedef std::tuple<Tuple, Equations::const_iterator> ApplicableTuple;
  typedef std::list<ApplicableTuple> applicable_list;
  applicable_list applicable;

  //find all the applicable ones

  Tuple::const_iterator iditer = k.find(DIM_ID);

  if (iditer != k.end())
  {
    try
    {
      //std::cerr << "looking for id: " <<
        //utf32_to_utf8(iditer->second.value<String>().value()) << std::endl;
      VariableMap::const_iterator viter =
        m_variables.find(iditer->second.value<String>().value());
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
        kp.erase(DIM_ID);
        return (*viter->second)(Tuple(kp));
      }
    }
    catch (std::bad_cast& e)
    {
      return TaggedConstant(Constant(Special(Special::DIMENSION),
                            TYPE_INDEX_SPECIAL), k);
    }
  }

  for (Equations::const_iterator eqn_i = m_equations.begin();
      eqn_i != m_equations.end(); ++eqn_i)
  {
    std::cerr << "best fitting" << std::endl;
    if (eqn_i->second.validContext())
    {
      try
      {
        std::cerr << "has valid context" << std::endl;
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
      std::cerr << "is applicable" << std::endl;
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

  applicable_list::const_iterator bestIter = applicable.end();

  for (applicable_list::const_iterator iter = applicable.begin();
       iter != applicable.end(); ++iter)
  {
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
  }

  if (bestIter == applicable.end())
  {
    return TaggedConstant(Constant(Special(Special::MULTIDEF),
                          TYPE_INDEX_SPECIAL), k);
  }

  //I suspect that this is buggy
  #if 0
  for (applicable_list::const_iterator iter = applicable.begin();
       iter != applicable.end(); ++iter)
  {
    if (std::get<1>(*bestIter)->second.equation()
          != std::get<1>(*iter)->second.equation()
        &&
        !tupleRefines(std::get<0>(*bestIter), std::get<0>(*iter)))
    {
      return TaggedConstant(Constant(Special(Special::MULTIDEF),
                            TYPE_INDEX_SPECIAL), k);
    }
  }
  #endif

  for (applicable_list::const_iterator iter = applicable.begin();
       iter != applicable.end(); ++iter)
  {
    if (iter != bestIter &&
        //std::get<1>(*bestIter)->second.equation()
        //  != std::get<1>(*iter)->second.equation()
        //&&
        !tupleRefines(std::get<0>(*bestIter), std::get<0>(*iter)))
    {
      return TaggedConstant(Constant(Special(Special::MULTIDEF),
                            TYPE_INDEX_SPECIAL), k);
    }
  }
  
  //std::cerr << "running equation " << std::get<1>(*bestIter)->id()
  //<< std::endl;
  return (*std::get<1>(*bestIter)->second.equation())(k);
}

std::pair<uuid, VariableHD::Equations::iterator>
VariableHD::addExprInternal(const Tuple& k, HD* e)
{
  size_t dim_id = DIM_ID;
  Tuple::const_iterator iter = k.find(dim_id);
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
      kp[dim_id] = Constant(String(end), TYPE_INDEX_USTRING);
    }
    else
    {
      kp.erase(dim_id);
    }

    return addToVariableActual(begin, Tuple(kp), e);
  }
}

std::pair<uuid, VariableHD::Equations::iterator>
VariableHD::addToVariableActual(const u32string& id, const Tuple& k, HD* h)
{
  //find the variable
  VariableMap::const_iterator iter = m_variables.find(id);
  if (iter == m_variables.end())
  {
    iter = m_variables.insert
    (
      std::make_pair(id, new VariableHD(id, m_system))
    ).first;
  }
  //add it
  return iter->second->addExprInternal(k, h);
}


bool
VariableHD::delexpr(uuid id, size_t time)
{
  Equations::iterator iter = m_equations.find(id);

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
  m_validEnd = time;
}

}
