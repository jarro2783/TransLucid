/* Workshops generated from AST::Expr.
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

/**
 * @file compiled_functors.cpp
 * The WSs that implement the evaluation of expressions.
 */

#include <gmpxx.h>

#include <tl/builtin_types.hpp>
#include <tl/chi.hpp>
#include <tl/context.hpp>
#include <tl/constws.hpp>
#include <tl/eval_workshops.hpp>
#include <tl/fixed_indexes.hpp>
#include <tl/internal_strings.hpp>
#include <tl/system.hpp>
#include <tl/types/demand.hpp>
#include <tl/types/dimension.hpp>
#include <tl/types/function.hpp>
#include <tl/types/region.hpp>
#include <tl/types/special.hpp>
#include <tl/types/tuple.hpp>
#include <tl/types/intension.hpp>
#include <tl/types_util.hpp>
#include <tl/utility.hpp>

#include <tl/output.hpp>

namespace TransLucid
{

namespace Workshops
{

namespace
{

  template <typename Iter, typename Transform>
  class tuple_transform_iterator : 
    public std::iterator
  <
    std::random_access_iterator_tag,
    std::pair<dimension_index, Constant>,
    typename std::iterator_traits<Iter>::difference_type
  >
  {
    typedef std::iterator
    <
      std::random_access_iterator_tag,
      std::pair<dimension_index, Constant>,
      typename std::iterator_traits<Iter>::difference_type
    > base;

    public:
    tuple_transform_iterator(Iter iter, Transform t) 
    : m_iter(iter) 
    , m_transform(t)
    {}

    tuple_transform_iterator&
    operator++()
    {
      ++m_iter;
      return *this;
    }

    typename base::value_type
    operator*() const
    {
      return m_transform(*m_iter);
    }

    typename base::difference_type
    operator-(const tuple_transform_iterator& rhs)
    {
      return m_iter - rhs.m_iter;
    }

    bool
    operator!=(const tuple_transform_iterator& rhs) const
    {
      return !(*this == rhs);
    }

    bool operator==(const tuple_transform_iterator& rhs) const
    {
      return this->m_iter == rhs.m_iter;
    }

    private:
    Iter m_iter;
    Transform m_transform;
  };

  template <typename Iter, typename Transform>
  tuple_transform_iterator<Iter, Transform>
  make_tuple_transform_iterator(Iter iter, Transform t)
  {
    return tuple_transform_iterator<Iter, Transform>(iter, t);
  }

}

DimensionWS::DimensionWS(System& system, dimension_index dim)
: m_value(Types::Dimension::create(dim))
{
}

DimensionWS::DimensionWS(System& system, const std::u32string& name)
: m_value(Types::Dimension::create(system.getDimensionIndex(name)))
{
}

IfWS::~IfWS()
{
  delete m_condition;
  delete m_then;
  delete m_else;

  for (auto p : m_elsifs_2)
  {
    delete p.first;
    delete p.second;
  }
}

Constant
HashSymbolWS::operator()(Context& k)
{
  return Types::Tuple::create(Tuple(k));
}

Constant
HashSymbolWS::operator()(Context& kappa, Context& delta)
{
  return Types::Tuple::create(Tuple(kappa));
}

TimeConstant
HashSymbolWS::operator()(Context& kappa, Delta& d, const Thread& w, size_t t)
{
  //one cannot return the whole context in this cached mode
  return TimeConstant(t, Types::Special::create(SP_ACCESS));
}

Constant
BoolConstWS::operator()(Context& k, Context& delta)
{
  return operator()(k);
}

Constant
BoolConstWS::operator()(Context& k)
{
  return m_value;
}

TimeConstant
BoolConstWS::operator()(Context& kappa, Delta& d, const Thread& w, size_t t)
{
  return TimeConstant(t, m_value);
}

Constant
TypeConstWS::operator()(Context& k, Context& delta)
{
  return operator()(k);
}

Constant
TypeConstWS::operator()(Context& k)
{
  return m_value;
}

TimeConstant
TypeConstWS::operator()(Context& kappa, Delta& d, const Thread& w, size_t t)
{
  return TimeConstant(t, m_value);
}

Constant
DimensionWS::operator()(Context& k)
{
  return m_value;
}

Constant
DimensionWS::operator()(Context& k, Context& delta)
{
  return operator()(delta);
}

TimeConstant
DimensionWS::operator()(Context& kappa, Delta& d, const Thread& w, size_t t)
{
  return TimeConstant(t, m_value);
}

Constant
IdentWS::operator()(Context& kappa, Context& delta)
{
  return evaluate(kappa, delta);
}

Constant
IdentWS::operator()(Context& k)
{
  return evaluate(k);
}

TimeConstant
IdentWS::operator()(Context& kappa, Delta& d, const Thread& w, size_t t)
{
  if (m_e == nullptr)
  {
    m_e = m_identifiers.lookup(m_name);
  }

  if (m_e != nullptr)
  {
    return (*m_e)(kappa, d, w, t);
  }
  else
  {
    return TimeConstant(t, Types::Special::create(SP_UNDEF));
  }

}

template <typename... Delta>
Constant
IdentWS::evaluate(Context& kappa, Delta&&... delta)
{
  //std::cerr << "evaluating variable: " << m_name << std::endl;
  if (m_e == nullptr)
  {
    m_e = m_identifiers.lookup(m_name);
  }

  if (m_e != nullptr)
  {
    return (*m_e)(kappa, delta...);
  }
  else
  {
    return Types::Special::create(SP_UNDEF);
  }
}

Constant
BangOpWS::operator()(Context& kappa, Context& delta)
{
  //lookup function in the system and call it

  //evaluate fn expr
  Constant fn = (*m_name)(kappa, delta);
  
  if (fn.index() == TYPE_INDEX_DEMAND)
  {
    return fn;
  }
  else if (fn.index() == TYPE_INDEX_BASE_FUNCTION)
  {
    bool isSpecial = false;
    bool isDemand = false;
    std::vector<Constant> args;
    for (auto ws : m_args)
    {
      args.push_back((*ws)(kappa, delta));

      if (args.back().index() == TYPE_INDEX_SPECIAL)
      {
        isSpecial = true;
      }

      if (args.back().index() == TYPE_INDEX_DEMAND)
      {
        isDemand = true;
      }
    }

    //deal with demands first
    if (isDemand)
    {
      std::vector<dimension_index> demands;
      for (const auto& c : args)
      {
        if (c.index() == TYPE_INDEX_DEMAND)
        {
          Types::Demand::append(c, demands);
        }
      }

      return Types::Demand::create(demands);
    }

    if (isSpecial)
    {
      //combine all the specials with the special combiner
      WS* combine = m_system.lookupIdentifiers().lookup(U"special_combine");
      if (combine == nullptr)
      {
        throw "no default special combiner";
      }

      Constant fn = (*combine)(kappa, delta);
      Constant currentValue;

      //find the first special
      auto iter = args.begin();
      while (iter->index() != TYPE_INDEX_SPECIAL)
      {
        ++iter;
      }

      currentValue = *iter;

      ++iter;
      while (iter != args.end())
      {
        if (iter->index() == TYPE_INDEX_SPECIAL)
        {
          Constant fn2 = applyFunction<FUN_VALUE>
            (kappa, delta, fn, currentValue);
          currentValue = applyFunction<FUN_VALUE>
            (kappa, delta, fn2, *iter);
        }
        ++iter;
      }

      return currentValue;
    }
    else
    {
      if (args.size() == 1)
      {
        //Constant arg = (*m_args[0])(kappa);
        return Types::BaseFunction::get(fn).apply(args.at(0));
      }
      else
      {
        return Types::BaseFunction::get(fn).apply(args);
      }
    }
  }
  else if (fn.index() == TYPE_INDEX_TUPLE && m_args.size() == 1)
  {
    //look up the appropriate dimension in the tuple
    Constant rhs = (*m_args[0])(kappa, delta);

    if (rhs.index() == TYPE_INDEX_DEMAND)
    {
      return rhs;
    }

    dimension_index dim = m_system.getDimensionIndex(rhs);
    const Tuple& lhs = Types::Tuple::get(fn);
    auto iter = lhs.find(dim);
    if (iter == lhs.end())
    {
      return Types::Special::create(SP_DIMENSION);
    }
    else
    {
      return iter->second;
    }
  }
  else if (m_args.size() == 1)
  {
    Constant rhs = (*m_args[0])(kappa, delta);

    System::IdentifierLookup lookup = m_system.lookupIdentifiers();
    WS* constant_bang = lookup.lookup(U"constant_bang");

    if (constant_bang == nullptr)
    {
      return Types::Special::create(SP_UNDEF);
    }

    Constant theFun = (*constant_bang)(kappa, delta);

    return applyFunction<FUN_VALUE>(kappa, delta,
      applyFunction<FUN_VALUE>(kappa, delta, theFun, fn),
      rhs
    );
  }
  else
  {
    return Types::Special::create(SP_UNDEF);
  }
}

Constant
BangOpWS::operator()(Context& k)
{
  //lookup function in the system and call it

  //evaluate fn expr
  RhoManager rho(k);
  Constant name = (*m_name)(k);

  if (name.index() == TYPE_INDEX_BASE_FUNCTION)
  {
    bool isSpecial = false;
    std::vector<Constant> args;
    int index = 1;
    for (auto ws : m_args)
    {
      rho.changeTop(index);
      args.push_back((*ws)(k));

      if (args.back().index() == TYPE_INDEX_SPECIAL)
      {
        isSpecial = true;
      }

      ++index;
    }

    if (isSpecial)
    {
      //combine all the specials with the special combiner
      WS* combine = m_system.lookupIdentifiers().lookup(U"special_combine");
      if (combine == nullptr)
      {
        throw "no default special combiner";
      }

      Constant fn = (*combine)(k);
      Constant currentValue;

      //find the first special
      auto iter = args.begin();
      while (iter->index() != TYPE_INDEX_SPECIAL)
      {
        ++iter;
      }

      currentValue = *iter;

      ++iter;
      while (iter != args.end())
      {
        if (iter->index() == TYPE_INDEX_SPECIAL)
        {
          Constant fn2 = applyFunction<FUN_VALUE>(k, fn, currentValue);
          currentValue = applyFunction<FUN_VALUE>(k, fn2, *iter);
        }
        ++iter;
      }

      return currentValue;
    }
    else
    {
      if (args.size() == 1)
      {
        Constant arg = (*m_args[0])(k);
        return Types::BaseFunction::get(name).apply(arg);
      }
      else
      {
        return Types::BaseFunction::get(name).apply(args);
      }
    }
  }
  else if (name.index() == TYPE_INDEX_TUPLE && m_args.size() == 1)
  {
    //look up the appropriate dimension in the tuple
    Constant rhs = (*m_args[0])(k);
    dimension_index dim = m_system.getDimensionIndex(rhs);
    const Tuple& lhs = Types::Tuple::get(name);
    auto iter = lhs.find(dim);
    if (iter == lhs.end())
    {
      return Types::Special::create(SP_DIMENSION);
    }
    else
    {
      return iter->second;
    }
  }
  else if (m_args.size() == 1)
  {
    Constant rhs = (*m_args[0])(k);

    System::IdentifierLookup lookup = m_system.lookupIdentifiers();
    WS* constant_bang = lookup.lookup(U"constant_bang");

    if (constant_bang == nullptr)
    {
      return Types::Special::create(SP_UNDEF);
    }

    Constant theFun = (*constant_bang)(k);

    return applyFunction<FUN_VALUE>(k,
      applyFunction<FUN_VALUE>(k, theFun, name),
      rhs
    );
  }
  else
  {
    return Types::Special::create(SP_UNDEF);
  }
}

TimeConstant
BangOpWS::operator()(Context& kappa, Delta& d, const Thread& w, size_t t)
{
  //lookup function in the system and call it

  //evaluate fn expr
  TimeConstant fn = (*m_name)(kappa, d, w, t);
  
  bool isSpecial = false;
  bool isDemand = fn.second.index() == TYPE_INDEX_DEMAND;
  std::vector<Constant> args;
  size_t maxTime = 0;
  for (auto ws : m_args)
  { 
    auto result = (*ws)(kappa, d, w, t);
    args.push_back(result.second);

    if (result.first > maxTime)
    {
      maxTime = result.first;
    }

    if (result.second.index() == TYPE_INDEX_SPECIAL)
    {
      isSpecial = true;
    }

    if (result.second.index() == TYPE_INDEX_DEMAND)
    {
      isDemand = true;
    }
  }

  if (isDemand)
  {
    //deal with demands first
    if (isDemand)
    {
      std::vector<dimension_index> demands;

      if (fn.second.index() == TYPE_INDEX_DEMAND)
      {
        Types::Demand::append(fn.second, demands);
      }

      for (const auto& c : args)
      {
        if (c.index() == TYPE_INDEX_DEMAND)
        {
          Types::Demand::append(c, demands);
        }
      }

      return TimeConstant(maxTime, Types::Demand::create(demands));
    }
  }
  
  else if (fn.second.index() == TYPE_INDEX_BASE_FUNCTION)
  {
    if (isSpecial)
    {
      //combine all the specials with the special combiner
      WS* combine = m_system.lookupIdentifiers().lookup(U"special_combine");
      if (combine == nullptr)
      {
        throw "no default special combiner";
      }

      TimeConstant fn = (*combine)(kappa, d, w, t);
      Constant currentValue;

      //find the first special
      auto iter = args.begin();
      while (iter->index() != TYPE_INDEX_SPECIAL)
      {
        ++iter;
      }

      currentValue = *iter;

      ++iter;
      while (iter != args.end())
      {
        if (iter->index() == TYPE_INDEX_SPECIAL)
        {
          Constant fn2 = applyFunction<FUN_VALUE>
            (kappa, d, w, t, fn.second, currentValue).second;
          currentValue = applyFunction<FUN_VALUE>
            (kappa, d, w, t, fn2, *iter).second;
        }
        ++iter;
      }

      return std::make_pair(maxTime, currentValue);
    }
    else
    {
      if (args.size() == 1)
      {
        //Constant arg = (*m_args[0])(kappa);
        return 
          Types::BaseFunction::get(fn.second).apply(args.at(0), d, w, maxTime);
      }
      else
      {
        return 
          Types::BaseFunction::get(fn.second).apply(args, d, w, maxTime);
      }
    }
  }
  else if (fn.second.index() == TYPE_INDEX_TUPLE && m_args.size() == 1)
  {
    dimension_index dim = m_system.getDimensionIndex(args[0]);
    const Tuple& lhs = Types::Tuple::get(fn.second);
    auto iter = lhs.find(dim);
    if (iter == lhs.end())
    {
      return std::make_pair(maxTime, Types::Special::create(SP_DIMENSION));
    }
    else
    {
      return std::make_pair(maxTime, iter->second);
    }
  }
  else if (m_args.size() == 1)
  {
    System::IdentifierLookup lookup = m_system.lookupIdentifiers();
    WS* constant_bang = lookup.lookup(U"constant_bang");

    if (constant_bang == nullptr)
    {
      return std::make_pair(maxTime, Types::Special::create(SP_UNDEF));
    }

    Constant theFun = (*constant_bang)(kappa, d, w, t).second;

    return std::make_pair(maxTime, applyFunction<FUN_VALUE>(kappa, d, w, t,
      applyFunction<FUN_VALUE>(kappa, d, w, t, theFun, fn.second).second,
      args[0]
    ).second);
  }
  else
  {
    return std::make_pair(maxTime, Types::Special::create(SP_UNDEF));
  }

  //keep the compiler happy
  return TimeConstant();
}

Constant
MakeIntenWS::operator()(Context& k)
{
  std::vector<Constant> binds;

  int index = 1;
  RhoManager rho(k);
  for (auto& b : m_binds)
  {
    rho.changeTop(index);
    binds.push_back((*b)(k));
    ++index;
  }

  return Types::Intension::create(&m_system, m_rhs, std::move(binds), 
    m_scope, k);
}

Constant
MakeIntenWS::operator()(Context& kappa, Context& delta)
{
  #ifdef CACHE_TODO
  #warning implement cache here
  #endif

  return Constant();
}

TimeConstant
MakeIntenWS::operator()(Context& kappa, Delta& d, const Thread& w, size_t t)
{
  std::vector<Constant> binds;
  std::vector<dimension_index> demands;

  size_t maxTime = 0;

  for (auto& b : m_binds)
  {
    auto r = (*b)(kappa, d, w, t);

    if (r.second.index() == TYPE_INDEX_DEMAND)
    {
      Types::Demand::append(r.second, demands);
    }
    else
    {
      binds.push_back(r.second);
    }

    maxTime = std::max(maxTime, r.first);
  }

  if (!demands.empty())
  {
    return std::make_pair(maxTime, Types::Demand::create(demands));
  }
  else
  {
    for (auto& c : binds)
    {
      auto i = m_system.getDimensionIndex(c);

      if (!d.contains(i))
      {
        demands.push_back(i);
      }
    }
    
    for (auto& c : m_scope)
    {
      if (!d.contains(c))
      {
        demands.push_back(c);
      }
    }

    if (!demands.empty())
    {
      return std::make_pair(maxTime, Types::Demand::create(demands));
    }

    return std::make_pair(maxTime,
      Types::Intension::create(&m_system, m_rhs, std::move(binds), 
        m_scope, kappa));
  }
}

Constant
EvalIntenWS::operator()(Context& k)
{
  RhoManager rho(k);
  Constant rhs = (*m_rhs)(k);

  if (rhs.index() != TYPE_INDEX_INTENSION)
  {
    return Types::Special::create(SP_TYPEERROR);
  }

  rho.changeTop(1);
  const IntensionType& inten = Types::Intension::get(rhs);
  return inten(k);
}

Constant
EvalIntenWS::operator()(Context& kappa, Context& delta)
{
  #ifdef CACHE_TODO
  #warning implement cache here
  #endif
  return Constant();
}

TimeConstant
EvalIntenWS::operator()(Context& kappa, Delta& d, const Thread& w, size_t t)
{
  auto rhs = (*m_rhs)(kappa, d, w, t);

  if (rhs.second.index() == TYPE_INDEX_DEMAND)
  {
    return rhs;
  }

  if (rhs.second.index() != TYPE_INDEX_INTENSION)
  {
    return std::make_pair(t, Types::Special::create(SP_TYPEERROR));
  }

  const IntensionType& inten = Types::Intension::get(rhs.second);
  return inten(kappa, d, w, t);
}

Constant
IfWS::operator()(Context& k)
{
  RhoManager rho(k);
  Constant condv = (*m_condition)(k);

  if (condv.index() == TYPE_INDEX_SPECIAL)
  {
    return condv;
  }
  else if (condv.index() == TYPE_INDEX_BOOL)
  {
    bool b = get_constant<bool>(condv);

    if (b)
    {
      rho.changeTop(1);
      return (*m_then)(k);
      //result = makeValue(e->then->visit(this, d));
    }
    else
    {
      uint8_t rhoIndex = 2;
      //run the elsifs and else
      for (const auto& p : m_elsifs_2)
      {
        rho.changeTop(rhoIndex);
        Constant cond = p.first->operator()(k);

        type_index index = cond.index();

        if (index == TYPE_INDEX_SPECIAL)
        {
          return cond;
        }
        else if (index == TYPE_INDEX_BOOL)
        {
          bool bcond = get_constant<bool>(cond);
          if (bcond)
          {
            rho.changeTop(rhoIndex + 1);
            return p.second->operator()(k);
          }
        }
        else
        {
          return Types::Special::create(SP_TYPEERROR);
        }

        rhoIndex += 2;
      }

      rho.changeTop(rhoIndex);
      return (*m_else)(k);
    }
  }
  else
  {
    return Types::Special::create(SP_TYPEERROR);
  }
}

Constant
IfWS::operator()(Context& kappa, Context& delta)
{
  Constant condv = (*m_condition)(kappa, delta);

  if (condv.index() == TYPE_INDEX_DEMAND)
  {
    return condv;
  }
  else if (condv.index() == TYPE_INDEX_SPECIAL)
  {
    return condv;
  }
  else if (condv.index() == TYPE_INDEX_BOOL)
  {
    bool b = get_constant<bool>(condv);

    if (b)
    {
      return (*m_then)(kappa, delta);
      //result = makeValue(e->then->visit(this, d));
    }
    else
    {
      //run the elsifs and else
      for (const auto& p : m_elsifs_2)
      {
        Constant cond = p.first->operator()(kappa, delta);

        type_index index = cond.index();

        if (index == TYPE_INDEX_DEMAND)
        {
          return cond;
        }
        else if (index == TYPE_INDEX_SPECIAL)
        {
          return cond;
        }
        else if (index == TYPE_INDEX_BOOL)
        {
          bool bcond = get_constant<bool>(cond);
          if (bcond)
          {
            return p.second->operator()(kappa, delta);
          }
        }
        else
        {
          return Types::Special::create(SP_TYPEERROR);
        }
      }

      return (*m_else)(kappa, delta);
    }
  }
  else
  {
    return Types::Special::create(SP_TYPEERROR);
  }
}

TimeConstant
IfWS::operator()(Context& kappa, Delta& d, const Thread& w, size_t t)
{
  auto condv = (*m_condition)(kappa, d, w, t);

  if (condv.second.index() == TYPE_INDEX_DEMAND)
  {
    return condv;
  }
  else if (condv.second.index() == TYPE_INDEX_SPECIAL)
  {
    return condv;
  }
  else if (condv.second.index() == TYPE_INDEX_BOOL)
  {
    bool b = get_constant<bool>(condv.second);

    if (b)
    {
      return (*m_then)(kappa, d, w, condv.first);
      //result = makeValue(e->then->visit(this, d));
    }
    else
    {
      //run the elsifs and else
      size_t maxTime = condv.first;
      for (const auto& p : m_elsifs_2)
      {
        auto cond = p.first->operator()(kappa, d, w, condv.first);

        type_index index = cond.second.index();

        if (index == TYPE_INDEX_DEMAND)
        {
          return cond;
        }
        else if (index == TYPE_INDEX_SPECIAL)
        {
          return cond;
        }
        else if (index == TYPE_INDEX_BOOL)
        {
          bool bcond = get_constant<bool>(cond.second);
          if (bcond)
          {
            return p.second->operator()(kappa, d, w, cond.first);
          }
        }
        else
        {
          return std::make_pair(cond.first, 
            Types::Special::create(SP_TYPEERROR));
        }

        maxTime = std::max(maxTime, cond.first);
      }

      return (*m_else)(kappa, d, w, maxTime);
    }
  }
  else
  {
    return std::make_pair(condv.first, Types::Special::create(SP_TYPEERROR));
  }
}

Constant
HashWS::operator()(Context& k)
{
  Constant r = (*m_e)(k);
  return lookup_context(m_system, r, k);
}

Constant
HashWS::operator()(Context& kappa, Context& delta)
{
  Constant r = (*m_e)(kappa, delta);

  if (r.index() == TYPE_INDEX_DEMAND)
  {
    return r;
  }

  if (m_cached)
  {
    return lookup_context_cached(m_system, r, delta);
  }
  else
  {
    return lookup_context(m_system, r, kappa);
  }
}

TimeConstant
HashWS::operator()(Context& kappa, Delta& d, const Thread& w, size_t t)
{
  auto r = (*m_e)(kappa, d, w, t);

  if (r.second.index() == TYPE_INDEX_DEMAND)
  {
    return r;
  }

  auto dim = m_system.getDimensionIndex(r.second);
  if (!d.contains(dim))
  {
    return std::make_pair(r.first, Types::Demand::create({dim}));
  }

  return std::make_pair(r.first, kappa.lookup(dim));
}

Constant
IntmpConstWS::operator()(Context& k)
{
  return m_value;
}

Constant
IntmpConstWS::operator()(Context& kappa, Context& delta)
{
  return operator()(kappa);
}

TimeConstant
IntmpConstWS::operator()(Context& kappa, Delta& d, const Thread& w, size_t t)
{
  return std::make_pair(t, operator()(kappa));
}

Constant
SpecialConstWS::operator()(Context& k)
{
  return Types::Special::create(m_value);
}

Constant
SpecialConstWS::operator()(Context& k, Context& delta)
{
  return operator()(k);
}

TimeConstant
SpecialConstWS::operator()(Context& kappa, Delta& d, const Thread& w, size_t t)
{
  return std::make_pair(t, operator()(kappa));
}

UStringConstWS::UStringConstWS(const u32string& s)
{
  auto iter = string_constants.find(s);

  if (iter != string_constants.end())
  {
    m_value = iter->second;
  }
  else
  {
    m_value = string_constants
      .insert(std::make_pair(s, Types::String::create(s))).first->second;
  }
}

Constant
UStringConstWS::operator()(Context& k)
{
  return m_value;
}

Constant
UStringConstWS::operator()(Context& k, Context& delta)
{
  return operator()(k);
}

TimeConstant
UStringConstWS::operator()(Context& kappa, Delta& d, const Thread& w, size_t t)
{
  return TimeConstant(t, m_value);
}

Constant
UCharConstWS::operator()(Context& k)
{
  return m_value;
}

Constant
UCharConstWS::operator()(Context& kappa, Context& delta)
{
  return operator()(kappa);
}

TimeConstant
UCharConstWS::operator()(Context& kappa, Delta& d, const Thread& w, size_t t)
{
  return TimeConstant(t, m_value);
}

Constant
RegionWS::operator()(Context& k)
{
  Region::Entries entries;

  for (const auto& entry : m_entries)
  {
    Constant lhs = (*std::get<0>(entry))(k);
    Constant rhs = (*std::get<2>(entry))(k);
    entries.insert(
      std::make_pair(
        m_system.getDimensionIndex(lhs), 
        std::make_pair(
          std::get<1>(entry), 
          rhs
        )
      )
    );
  }

  return Types::Region::create(Region(entries));
}

Constant
RegionWS::operator()(Context& kappa, Context& delta)
{
  return Constant();
}

TimeConstant
RegionWS::operator()(Context& kappa, Delta& d, const Thread& w, size_t t)
{
  Region::Entries entries;

  std::vector<dimension_index> demands;
  size_t maxTime = 0;

  for (const auto& entry : m_entries)
  {
    auto lhs = (*std::get<0>(entry))(kappa, d, w, t);
    auto rhs = (*std::get<2>(entry))(kappa, d, w, t);

    maxTime = std::max(maxTime, lhs.first);
    maxTime = std::max(maxTime, rhs.first);

    if (lhs.second.index() == TYPE_INDEX_DEMAND)
    {
      Types::Demand::append(lhs.second, demands);
    }

    if (rhs.second.index() == TYPE_INDEX_DEMAND)
    {
      Types::Demand::append(rhs.second, demands);
    }

    entries.insert(
      std::make_pair(
        m_system.getDimensionIndex(lhs.second), 
        std::make_pair(
          std::get<1>(entry), 
          rhs.second
        )
      )
    );
  }

  if (!demands.empty())
  {
    return std::make_pair(maxTime, Types::Demand::create(demands));
  }

  return std::make_pair(maxTime, Types::Region::create(Region(entries)));
}

Constant
TupleWS::operator()(Context& k)
{
  uint8_t index = 0;
  RhoManager rho(k);
  tuple_t kp;
  for(auto& pair : m_elements)
  {
    rho.changeTop(index * 2);
    //const Pair& p = v.first.value<Pair>();
    Constant left = (*pair.first)(k);
    rho.changeTop(index * 2 + 1);
    Constant right = (*pair.second)(k);
    
    if (left.index() == TYPE_INDEX_SPECIAL)
    {
      return left;
    }
    else if (right.index() == TYPE_INDEX_SPECIAL)
    {
      return right;
    }
    else if (left.index() == TYPE_INDEX_DIMENSION)
    {
      kp[get_constant<dimension_index>(left)] = right;
    }
    else
    {
      kp[m_system.getDimensionIndex(left)] = right;
    }

    ++index;
  }
  return Types::Tuple::create(Tuple(kp));
}

Constant
TupleWS::operator()(Context& kappa, Context& delta)
{
  std::vector<dimension_index> demands;
  tuple_t kp;
  for(auto& pair : m_elements)
  {
    bool hasdemands = false;
    //const Pair& p = v.first.value<Pair>();
    Constant left = (*pair.first)(kappa, delta);
    Constant right = (*pair.second)(kappa, delta);

    if (left.index() == TYPE_INDEX_DEMAND)
    {
      Types::Demand::append(left, demands);
      hasdemands = true;
    }

    if (right.index() == TYPE_INDEX_DEMAND)
    {
      Types::Demand::append(right, demands);
      hasdemands = true;
    }

    if (!hasdemands)
    {
      if (left.index() == TYPE_INDEX_SPECIAL)
      {
        return left;
      }

      if (left.index() == TYPE_INDEX_SPECIAL)
      {
        return left;
      }
      else if (right.index() == TYPE_INDEX_SPECIAL)
      {
        return right;
      }

      if (left.index() == TYPE_INDEX_DIMENSION)
      {
        kp[get_constant<dimension_index>(left)] = right;
      }
      else
      {
        kp[m_system.getDimensionIndex(left)] = right;
      }
    }
  }

  if (demands.size() == 0)
  {
    return Types::Tuple::create(Tuple(kp));
  }
  else
  {
    return Types::Demand::create(demands);
  }
}

TimeConstant
TupleWS::operator()(Context& kappa, Delta& d, const Thread& w, size_t t)
{
  std::vector<dimension_index> demands;
  tuple_t kp;
  size_t maxTime = 0;

  for(auto& pair : m_elements)
  {
    bool hasdemands = false;
    //const Pair& p = v.first.value<Pair>();
    auto left = (*pair.first)(kappa, d, w, t);
    auto right = (*pair.second)(kappa, d, w, t);

    maxTime = std::max(maxTime, left.first);
    maxTime = std::max(maxTime, right.first);

    if (left.second.index() == TYPE_INDEX_DEMAND)
    {
      Types::Demand::append(left.second, demands);
      hasdemands = true;
    }

    if (right.second.index() == TYPE_INDEX_DEMAND)
    {
      Types::Demand::append(right.second, demands);
      hasdemands = true;
    }

    if (!hasdemands)
    {
      if (left.second.index() == TYPE_INDEX_SPECIAL)
      {
        return std::make_pair(maxTime, left.second);
      }

      if (left.second.index() == TYPE_INDEX_SPECIAL)
      {
        return std::make_pair(maxTime, left.second);
      }
      else if (right.second.index() == TYPE_INDEX_SPECIAL)
      {
        return std::make_pair(maxTime, right.second);
      }

      if (left.second.index() == TYPE_INDEX_DIMENSION)
      {
        kp[get_constant<dimension_index>(left.second)] = right.second;
      }
      else
      {
        kp[m_system.getDimensionIndex(left.second)] = right.second;
      }
    }
  }

  if (demands.size() == 0)
  {
    return std::make_pair(maxTime, Types::Tuple::create(Tuple(kp)));
  }
  else
  {
    return std::make_pair(maxTime, Types::Demand::create(demands));
  }
}

Constant
AtWS::operator()(Context& k)
{
  //tuple_t kNew = k.tuple();
  RhoManager rho(k, 1);
  Constant val1 = (*e1)(k);
  if (val1.index() != TYPE_INDEX_TUPLE)
  {
    return Types::Special::create(SP_TYPEERROR);
  }
  else
  {
    //validate time
    auto& t = Types::Tuple::get(val1);
    auto& delta = t.tuple();
    const auto& dimTime = delta.find(DIM_TIME);
    if (dimTime != delta.end() && Types::Intmp::get(dimTime->second) > 
      Types::Intmp::get(k.lookup(DIM_TIME)))
    {
      return Types::Special::create(SP_ACCESS);
    }

    ContextPerturber p(k, t);
    rho.changeTop(2);
    return (*e2)(k);
  }
}

Constant
AtWS::operator()(Context& kappa, Context& delta)
{
  //tuple_t kNew = k.tuple();
  Constant val1 = (*e1)(kappa, delta);

  if (val1.index() == TYPE_INDEX_DEMAND)
  {
    return val1;
  }

  if (val1.index() != TYPE_INDEX_TUPLE)
  {
    return Types::Special::create(SP_TYPEERROR);
  }
  else
  {
    //validate time
    auto& t = Types::Tuple::get(val1);
    auto& change = t.tuple();
    const auto& dimTime = change.find(DIM_TIME);
    if (dimTime != change.end() && Types::Intmp::get(dimTime->second) > 
      Types::Intmp::get(kappa.lookup(DIM_TIME)))
    {
      return Types::Special::create(SP_ACCESS);
    }

    ContextPerturber pkappa(kappa, t);
    ContextPerturber pdelta(delta, t);
    return (*e2)(kappa, delta);
  }
}

TimeConstant
AtWS::operator()(Context& kappa, Delta& d, const Thread& w, size_t t)
{
  auto rhs = (*e1)(kappa, d, w, t);

  if (rhs.second.index() == TYPE_INDEX_DEMAND)
  {
    return rhs;
  }

  if (rhs.second.index() != TYPE_INDEX_TUPLE)
  {
    return std::make_pair(rhs.first, Types::Special::create(SP_TYPEERROR));
  }

  auto& tuple = Types::Tuple::get(rhs.second);
  
  //validate the time dimension
  auto& change = tuple.tuple();
  const auto& dimTime = change.find(DIM_TIME);
  if (dimTime != change.end() && Types::Intmp::get(dimTime->second) > 
    Types::Intmp::get(kappa.lookup(DIM_TIME)))
  {
    return std::make_pair(rhs.first, Types::Special::create(SP_ACCESS));
  }

  ContextPerturber pk(kappa, tuple);
  DeltaPerturber pd(d);
  pd.perturb(change);

  return (*e2)(kappa, d, w, rhs.first);
}

Constant
HostOpWS::operator()(Context& k)
{
  if (m_function == nullptr)
  {
    m_function = m_system.lookupBaseFunction(m_name);
  }

  if (m_function == nullptr)
  {
    return Types::Special::create(SP_CONST);
  }
  else
  {
    if (m_function->arity() == 0)
    {
      return m_function->apply(std::vector<Constant>());
    }
    else
    {
      return Types::BaseFunction::create(*m_function);
    }
  }
}

Constant
HostOpWS::operator()(Context& kappa, Context& delta)
{
  return operator()(kappa);
}

TimeConstant
HostOpWS::operator()(Context& kappa, Delta& d, const Thread& w, size_t t)
{
  return std::make_pair(t, operator()(kappa));
}

Constant
BaseAbstractionWS::operator()(Context& k)
{
  return Types::BaseFunction::create
  (
    BaseFunctionAbstraction
    (
      m_system,
      m_dims,
      m_scope,
      m_binds,
      m_rhs,
      k
    )
  );
}

Constant
BaseAbstractionWS::operator()(Context& kappa, Context& delta)
{
  return Constant();
}

TimeConstant
BaseAbstractionWS::operator()
(Context& kappa, Delta& d, const Thread& w, size_t t)
{
  std::vector<dimension_index> binds;
  std::vector<dimension_index> demands;
  size_t maxTime = 0;

  for (auto b : m_binds)
  {
    auto result = (*b)(kappa, d, w, t);
    maxTime = std::max(maxTime, result.first);

    if (result.second.index() == TYPE_INDEX_DEMAND)
    {
      Types::Demand::append(result.second, demands);
    }
    else
    {
      binds.push_back(m_system->getDimensionIndex(result.second));
    }
  }

  if (!demands.empty())
  {
    return std::make_pair(maxTime, Types::Demand::create(demands));
  }

  //are the bound dimensions in the context?
  for (auto dim : binds)
  {
    if (!d.contains(dim))
    {
      demands.push_back(dim);
    }
  }

  for (auto dim : m_scope)
  {
    if (!d.contains(dim))
    {
      demands.push_back(dim);
    }
  }

  if (!demands.empty())
  {
    return std::make_pair(maxTime, Types::Demand::create(demands));
  }

  return std::make_pair(maxTime, Types::BaseFunction::create(
    BaseFunctionAbstraction
    (
      m_dims,
      m_scope,
      binds,
      m_rhs,
      kappa
    )));
}

Constant
LambdaAbstractionWS::operator()(Context& k)
{
  return createValueFunction
    (
      m_system,
      m_name,
      m_argDim,
      m_rhs,
      m_binds,
      m_scope,
      k
    );

  #if 0
  return Types::ValueFunction::create
  (
    ValueFunctionType
    (
      m_system,
      m_name, 
      m_argDim, 
      m_scope,
      m_free,
      m_rhs, 
      k
    )
  );
  #endif
}

Constant
LambdaAbstractionWS::operator()(Context& kappa, Context& delta)
{
  //but evaluating the free variables needs to be done in a cached manner
  return createValueFunctionCached
    (
      m_system,
      m_name,
      m_argDim,
      m_rhs,
      kappa,
      delta
    );

  #if 0
  return Types::ValueFunction::create
  (
    ValueFunctionType
    (
      m_system,
      m_name, 
      m_argDim, 
      m_scope,
      m_free,
      m_rhs, 
      k
    )
  );
  #endif
}

TimeConstant
LambdaAbstractionWS::operator() 
(Context& kappa, Delta& d, const Thread& w, size_t t)
{
  return createValueFunctionCachedNew
    (
      m_system,
      m_name,
      m_argDim,
      m_rhs,
      m_binds,
      m_scope,
      kappa,
      d, w, t
    );
}

Constant
LambdaApplicationWS::operator()(Context& k)
{
  //evaluate the lhs, evaluate the rhs
  //and pass the value to the function to evaluate

  RhoManager rho(k);
  rho.changeTop(1);
  Constant lhs = (*m_lhs)(k);
  //first make sure that it is a function
  if (lhs.index() != TYPE_INDEX_VALUE_FUNCTION)
  {
    return Types::Special::create(SP_TYPEERROR);
  }

  rho.changeTop(2);
  Constant rhs = (*m_rhs)(k);
  const ValueFunctionType& f = Types::ValueFunction::get(lhs);

  rho.changeTop(3);
  return f.apply(k, rhs);
}

Constant
LambdaApplicationWS::operator()(Context& kappa, Context& delta)
{
  //evaluate the lhs, evaluate the rhs
  //and pass the value to the function to evaluate

  Constant lhs = (*m_lhs)(kappa, delta);
  //first make sure that it is a function
  if (lhs.index() == TYPE_INDEX_DEMAND)
  {
    return lhs;
  }
  else if (lhs.index() != TYPE_INDEX_VALUE_FUNCTION)
  {
    return Types::Special::create(SP_TYPEERROR);
  }

  Constant rhs = (*m_rhs)(kappa, delta);

  if (rhs.index() == TYPE_INDEX_DEMAND)
  {
    return rhs;
  }

  const ValueFunctionType& f = Types::ValueFunction::get(lhs);

  return f.apply(kappa, delta, rhs);
}

TimeConstant
LambdaApplicationWS::operator()
(Context& kappa, Delta& d, const Thread& w, size_t t)
{
  auto lhs = (*m_lhs)(kappa, d, w, t);
  auto rhs = (*m_rhs)(kappa, d, w, t);

  std::vector<dimension_index> demands;

  if (lhs.second.index() == TYPE_INDEX_DEMAND)
  {
    Types::Demand::append(lhs.second, demands);
  }

  if (rhs.second.index() == TYPE_INDEX_DEMAND)
  {
    Types::Demand::append(rhs.second, demands);
  }

  if (!demands.empty())
  {
    return std::make_pair(std::max(lhs.first, rhs.first), 
      Types::Demand::create(demands));
  }

  if (lhs.second.index() != TYPE_INDEX_VALUE_FUNCTION)
  {
    return std::make_pair(lhs.first, Types::Special::create(SP_TYPEERROR));
  }

  const ValueFunctionType& f = Types::ValueFunction::get(lhs.second);

  return f.apply(kappa, d, w, std::max(lhs.first, rhs.first), rhs.second);
}

Constant
WhereWS::operator()(Context& k)
{
  if (m_simplified)
  {
    //this is much the same as the cache one, but without the cache
    //we use the actual dimension allocated, and insert it into the context
    std::vector<std::pair<dimension_index, Constant>> change;

    for (const auto& v : m_dims)
    {
      change.push_back(std::make_pair(v.first, (*v.second)(k)));
    }

    ContextPerturber p(k, change);

    return (*m_expr)(k);
  }
  else
  {
    //get a new CHI dimension for each dim
    //evaluate each initialiser
    //perturb theta_j with the initiliaser
    //evaluate E_0

    RhoManager rho(k);

    std::vector<std::pair<dimension_index, Constant>> change;

    int index = 1;
    for (const auto& v : m_dims)
    {
      //the CHI dimension
      ChiDim chi(index, 
        std::vector<ChiDim::type_t>(k.getRho().begin(), k.getRho().end()));
      dimension_index d = m_system.getChiDim(chi);

      //std::cerr << "chi dimension: " << chi << " has index " << d << std::endl;

      //the initialiser
      if (v.second != nullptr)
      {
        rho.changeTop(index);
        change.push_back(std::make_pair(d, (*v.second)(k)));

        //std::cerr << "setting dimension " << d << std::endl;
      }

      //which dim are we actually using
      //std::cerr << "setting: [" << v.first << " <- " << d << "]" << std::endl;
      change.push_back(std::make_pair(v.first, Types::Dimension::create(d)));
      ++index;
    }

    rho.changeTop(0);
    ContextPerturber p(k, change);
    return (*m_expr)(k);
  }
}

Constant
WhereWS::operator()(Context& kappa, Context& delta)
{
  return Constant();
}

TimeConstant
WhereWS::operator()(Context& kappa, Delta& d, const Thread& w, size_t t)
{
  std::vector<dimension_index> demands;
  std::vector<std::pair<dimension_index, Constant>> change;
  size_t maxTime = 0;

  //setup chi depth
  int depth = 0;
  if (kappa.has_entry(m_psiQ))
  {
    depth = Types::Intmp::get(kappa.lookup(m_psiQ)).get_si();
  }

  m_chivalue.push_back(depth);

  int index = 0;
  for (auto v : m_dims)
  {
    auto init = (*v.second)(kappa, d, w, t);

    //allocate cached chi dim
    ChiDim chi(index, m_chivalue);
    auto dimIndex = m_system.getChiDim(chi);

    change.push_back(std::make_pair(dimIndex, init.second));
    change.push_back(std::make_pair(v.first, 
      Types::Dimension::create(dimIndex)));

    maxTime = std::max(maxTime, init.first);

    if (init.second.index() == TYPE_INDEX_DEMAND)
    {
      Types::Demand::append(init.second, demands);
    }

    ++index;
  }

  //restore chi depth
  m_chivalue.pop_back();

  if (!demands.empty())
  {
    return std::make_pair(maxTime, Types::Demand::create(demands));
  }

  change.push_back(std::make_pair(m_psiQ, Types::Intmp::create(depth + 1)));

  ContextPerturber pk(kappa, change);
  DeltaPerturber pd(d);
  pd.perturb(change);

  return (*m_expr)(kappa, d, w, t);
}

Constant
AtTupleWS::operator()(Context& k)
{
  //evaluate the tuple into a vector of fixed size
  //do some magic to initialise the vector with iterators that do the
  //evaluation all at once so that we only need one allocation

  RhoManager rho(k);
  int index = 1;

  bool access = false;

  const Constant& dimTime = k.lookup(DIM_TIME);

  std::vector<std::pair<dimension_index, Constant>> tuple;
  std::vector<Constant> specials;

  for (const auto& entry : m_tuple)
  {
    rho.changeTop(index*2);
    Constant lhs = (*entry.first)(k);
    rho.changeTop(index*2 + 1);
    Constant rhs = (*entry.second)(k);

    if (lhs.index() == TYPE_INDEX_SPECIAL)
    {
      specials.push_back(lhs);
    }
    else if (rhs.index() == TYPE_INDEX_SPECIAL)
    {
      specials.push_back(rhs);
    }
    else
    {
      if (lhs.index() == TYPE_INDEX_DIMENSION)
      {
        if (get_constant<dimension_index>(lhs) == DIM_TIME &&
            Types::Intmp::get(rhs) > Types::Intmp::get(dimTime))
        {
          access = true;
        }

        tuple.push_back(
          std::make_pair(get_constant<dimension_index>(lhs), rhs));
      }
      else
      {
        tuple.push_back(
          std::make_pair(this->m_system.getDimensionIndex(lhs), rhs));
      }
    }

    ++index;
  }
  
  if (specials.size() > 0)
  {
    return specials.at(0);
  }

  if (access)
  {
    return Types::Special::create(SP_ACCESS);
  }
  else
  {
    ContextPerturber p(k, tuple);
    rho.changeTop(1);
    return (*m_e2)(k);
  }
}

Constant
AtTupleWS::operator()(Context& kappa, Context& delta)
{
  //evaluate the tuple into a vector of fixed size
  //do some magic to initialise the vector with iterators that do the
  //evaluation all at once so that we only need one allocation

  bool access = false;

  const Constant& dimTime = kappa.lookup(DIM_TIME);

  std::vector<std::pair<dimension_index, Constant>> tuple;
  std::set<dimension_index> demands;
  std::vector<Constant> specials;

  for (const auto& entry : m_tuple)
  {
    Constant lhs = (*entry.first)(kappa, delta);
    Constant rhs = (*entry.second)(kappa, delta);

    if (lhs.index() == TYPE_INDEX_SPECIAL)
    {
      specials.push_back(lhs);
    }

    if (rhs.index() == TYPE_INDEX_SPECIAL)
    {
      specials.push_back(rhs);
    }

    if (lhs.index() == TYPE_INDEX_DEMAND || rhs.index() == TYPE_INDEX_DEMAND)
    {
      if (lhs.index() == TYPE_INDEX_DEMAND)
      {
        Types::Demand::append(lhs, demands);
      }

      if (rhs.index() == TYPE_INDEX_DEMAND)
      {
        Types::Demand::append(rhs, demands);
      }
    }
    else
    {
      if (lhs.index() == TYPE_INDEX_DIMENSION)
      {
        if (get_constant<dimension_index>(lhs) == DIM_TIME &&
            Types::Intmp::get(rhs) > Types::Intmp::get(dimTime))
        {
          access = true;
        }

        tuple.push_back(
          std::make_pair(get_constant<dimension_index>(lhs), rhs));
      }
      else
      {
        tuple.push_back(
          std::make_pair(this->m_system.getDimensionIndex(lhs), rhs));
      }
    }
  }

  if (demands.size() != 0)
  {
    return Types::Demand::create(
      std::vector<dimension_index>(demands.begin(), demands.end()));
  }

  if (specials.size() > 0)
  {
    return specials.at(0);
  }

  if (access)
  {
    return Types::Special::create(SP_ACCESS);
  }
  else
  {
    ContextPerturber pkappa(kappa, tuple);
    ContextPerturber pdelta(delta, tuple);
    return (*m_e2)(kappa, delta);
  }
}

AtTupleWS::~AtTupleWS()
{
  delete m_e2;
  for (auto p : m_tuple)
  {
    delete p.first;
    delete p.second;
  }
}

TimeConstant
AtTupleWS::operator()(Context& kappa, Delta& d, const Thread& w, size_t t)
{
  //evaluate the tuple into a vector of fixed size

  bool access = false;

  const Constant& dimTime = kappa.lookup(DIM_TIME);

  std::vector<std::pair<dimension_index, Constant>> tuple;
  std::set<dimension_index> demands;
  std::vector<Constant> specials;
  size_t maxTime = 0;

  for (const auto& entry : m_tuple)
  {
    auto lhs = (*entry.first)(kappa, d, w, t);
    auto rhs = (*entry.second)(kappa, d, w, t);

    maxTime = std::max(lhs.first, maxTime);
    maxTime = std::max(rhs.first, maxTime);

    if (lhs.second.index() == TYPE_INDEX_SPECIAL)
    {
      specials.push_back(lhs.second);
    }

    if (rhs.second.index() == TYPE_INDEX_SPECIAL)
    {
      specials.push_back(rhs.second);
    }

    if (lhs.second.index() == TYPE_INDEX_DEMAND || 
        rhs.second.index() == TYPE_INDEX_DEMAND)
    {
      if (lhs.second.index() == TYPE_INDEX_DEMAND)
      {
        Types::Demand::append(lhs.second, demands);
      }

      if (rhs.second.index() == TYPE_INDEX_DEMAND)
      {
        Types::Demand::append(rhs.second, demands);
      }
    }
    else
    {
      if (lhs.second.index() == TYPE_INDEX_DIMENSION)
      {
        if (get_constant<dimension_index>(lhs.second) == DIM_TIME &&
            Types::Intmp::get(rhs.second) > Types::Intmp::get(dimTime))
        {
          access = true;
        }

        tuple.push_back(
          std::make_pair(
            get_constant<dimension_index>(lhs.second), rhs.second));
      }
      else
      {
        tuple.push_back(
          std::make_pair(this->m_system.getDimensionIndex(lhs.second), 
            rhs.second));
      }
    }
  }

  if (demands.size() != 0)
  {
    return std::make_pair(maxTime, Types::Demand::create(
      std::vector<dimension_index>(demands.begin(), demands.end())));
  }

  if (specials.size() > 0)
  {
    return std::make_pair(maxTime, specials.at(0));
  }

  if (access)
  {
    return std::make_pair(maxTime, Types::Special::create(SP_ACCESS));
  }
  else
  {
    ContextPerturber pkappa(kappa, tuple);
    DeltaPerturber pdelta(d);
    pdelta.perturb(tuple);
    return (*m_e2)(kappa, d, w, maxTime);
  }
}

} //namespace Workshops

} //namespace TransLucid
