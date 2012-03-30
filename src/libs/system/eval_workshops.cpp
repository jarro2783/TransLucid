/* Workshops generated from AST::Expr.
   Copyright (C) 2009--2012 Jarryd Beck and John Plaice

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
#include <tl/context.hpp>
#include <tl/constws.hpp>
#include <tl/eval_workshops.hpp>
#include <tl/fixed_indexes.hpp>
#include <tl/internal_strings.hpp>
#include <tl/system.hpp>
#include <tl/types/demand.hpp>
#include <tl/types/dimension.hpp>
#include <tl/types/function.hpp>
#include <tl/types/special.hpp>
#include <tl/types/tuple.hpp>
#include <tl/types/workshop.hpp>
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

template <typename... Delta>
Constant
IdentWS::evaluate(Context& kappa, Delta&&... delta)
{
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

  //evaluate name expr
  Constant name = (*m_name)(kappa, delta);

  if (name.index() == TYPE_INDEX_BASE_FUNCTION)
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
          Constant fn2 = applyFunction(kappa, delta, fn, currentValue);
          currentValue = applyFunction(kappa, delta, fn2, *iter);
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
        return Types::BaseFunction::get(name).apply(args.at(0));
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
    Constant rhs = (*m_args[0])(kappa, delta);
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
  else if (name.index() == TYPE_INDEX_DEMAND)
  {
    return name;
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

  //evaluate name expr
  Constant name = (*m_name)(k);

  if (name.index() == TYPE_INDEX_BASE_FUNCTION)
  {
    bool isSpecial = false;
    std::vector<Constant> args;
    for (auto ws : m_args)
    {
      args.push_back((*ws)(k));

      if (args.back().index() == TYPE_INDEX_SPECIAL)
      {
        isSpecial = true;
      }
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
          Constant fn2 = applyFunction(k, fn, currentValue);
          currentValue = applyFunction(k, fn2, *iter);
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
  else
  {
    return Types::Special::create(SP_UNDEF);
  }
}

Constant
IfWS::operator()(Context& k)
{
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
      return (*m_then)(k);
      //result = makeValue(e->then->visit(this, d));
    }
    else
    {
      //run the elsifs and else
      for (const auto& p : m_elsifs_2)
      {
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
            return p.second->operator()(k);
          }
        }
        else
        {
          return Types::Special::create(SP_TYPEERROR);
        }
      }

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
  return lookup_context_cached(m_system, r, delta);
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

Constant
TupleWS::operator()(Context& k)
{
  tuple_t kp;
  for(auto& pair : m_elements)
  {
    //const Pair& p = v.first.value<Pair>();
    Constant left = (*pair.first)(k);
    Constant right = (*pair.second)(k);

    if (left.index() == TYPE_INDEX_DIMENSION)
    {
      kp[get_constant<dimension_index>(left)] = right;
    }
    else if (left.index() == TYPE_INDEX_SPECIAL)
    {
      return left;
    }
    else
    {
      kp[m_system.getDimensionIndex(left)] = right;
    }
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

Constant
AtWS::operator()(Context& k)
{
  //tuple_t kNew = k.tuple();
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

Constant
BaseAbstractionWS::operator()(Context& k)
{
  return Types::BaseFunction::create
  (
    BaseFunctionAbstraction
    (
      //m_name, 
      m_argDim, 
      m_scope,
      m_rhs, 
      k
    )
  );
}

Constant
LambdaAbstractionWS::operator()(Context& k)
{
  return createValueFunction
    (
      m_system,
      m_name,
      m_argDim,
      m_scope,
      m_free,
      m_rhs,
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
  //first verify that m_scope are all in delta

  std::vector<dimension_index> demands;

  //but evaluating the free variables needs to be done in a cached manner
  return createValueFunctionCached
    (
      m_system,
      m_name,
      m_argDim,
      m_scope,
      m_free,
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

Constant
LambdaApplicationWS::operator()(Context& k)
{
  //evaluate the lhs, evaluate the rhs
  //and pass the value to the function to evaluate

  Constant lhs = (*m_lhs)(k);
  //first make sure that it is a function
  if (lhs.index() != TYPE_INDEX_VALUE_FUNCTION)
  {
    return Types::Special::create(SP_TYPEERROR);
  }

  Constant rhs = (*m_rhs)(k);
  const ValueFunctionType& f = Types::ValueFunction::get(lhs);

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

Constant
NamedAbstractionWS::operator()(Context& k)
{
  return createNameFunction
    (
      m_system, 
      m_name, 
      m_argDim, 
      m_odometerDim, 
      m_scope, 
      m_free, 
      m_rhs, 
      k
    );

  #if 0
  return Types::NameFunction::create
  (
    NameFunctionType
    (
      m_system,
      m_name,
      m_argDim,
      m_odometerDim,
      m_scope,
      m_free,
      m_rhs, 
      k
    )
  );
  #endif
}

Constant
NamedAbstractionWS::operator()(Context& kappa, Context& delta)
{
  return createNameFunctionCached
    (
      m_system, 
      m_name, 
      m_argDim, 
      m_odometerDim, 
      m_scope, 
      m_free, 
      m_rhs, 
      kappa,
      delta
    );
}

Constant
NameApplicationWS::operator()(Context& k)
{
  Constant lhs = (*m_lhs)(k);

  if (lhs.index() != TYPE_INDEX_NAME_FUNCTION)
  {
    return Types::Special::create(SP_TYPEERROR);
  }

  //named application passes a pointer to the intension
  Constant rhs = Types::Workshop::create(m_rhs);
  const NameFunctionType& f = Types::NameFunction::get(lhs);

  return f.apply(k, rhs, m_Lall);
}

Constant
NameApplicationWS::operator()(Context& kappa, Context& delta)
{
  Constant lhs = (*m_lhs)(kappa, delta);

  if (lhs.index() == TYPE_INDEX_DEMAND)
  {
    return lhs;
  }

  if (lhs.index() != TYPE_INDEX_NAME_FUNCTION)
  {
    return Types::Special::create(SP_TYPEERROR);
  }

  //named application passes a pointer to the intension
  Constant rhs = Types::Workshop::create(m_rhs);
  const NameFunctionType& f = Types::NameFunction::get(lhs);

  return f.apply(kappa, delta, rhs, m_Lall);
}
Constant
AtTupleWS::operator()(Context& k)
{
  //evaluate the tuple into a vector of fixed size
  //do some magic to initialise the vector with iterators that do the
  //evaluation all at once so that we only need one allocation

  bool access = false;

  const Constant& dimTime = k.lookup(DIM_TIME);

  auto evalTuple = [this, &k, &access, &dimTime] 
    (const std::pair<WS*, WS*>& entry)
    -> std::pair<dimension_index, Constant>
  {
    Constant lhs = (*entry.first)(k);
    Constant rhs = (*entry.second)(k);

    if (lhs.index() == TYPE_INDEX_DIMENSION)
    {
      if (get_constant<dimension_index>(lhs) == DIM_TIME &&
          Types::Intmp::get(rhs) > Types::Intmp::get(dimTime))
      {
        access = true;
      }

      return std::make_pair(get_constant<dimension_index>(lhs), rhs);
    }
    else
    {
      return std::make_pair(this->m_system.getDimensionIndex(lhs), rhs);
    }

  }
  ;

  std::vector<std::pair<dimension_index, Constant>> tuple
  (
    make_tuple_transform_iterator(m_tuple.begin(), evalTuple),
    make_tuple_transform_iterator(m_tuple.end(), evalTuple)
  );

  if (access)
  {
    return Types::Special::create(SP_ACCESS);
  }
  else
  {
    ContextPerturber p(k, tuple);
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

  auto evalTuple = [this, &kappa, &delta, &access, &dimTime] 
    (const std::pair<WS*, WS*>& entry)
    -> std::pair<dimension_index, Constant>
  {
    Constant lhs = (*entry.first)(kappa, delta);
    Constant rhs = (*entry.second)(kappa, delta);

    if (lhs.index() == TYPE_INDEX_DIMENSION)
    {
      if (get_constant<dimension_index>(lhs) == DIM_TIME &&
          Types::Intmp::get(rhs) > Types::Intmp::get(dimTime))
      {
        access = true;
      }

      return std::make_pair(get_constant<dimension_index>(lhs), rhs);
    }
    else
    {
      return std::make_pair(this->m_system.getDimensionIndex(lhs), rhs);
    }

  }
  ;

  std::vector<std::pair<dimension_index, Constant>> tuple
  (
    make_tuple_transform_iterator(m_tuple.begin(), evalTuple),
    make_tuple_transform_iterator(m_tuple.end(), evalTuple)
  );

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

} //namespace Workshops

} //namespace TransLucid
