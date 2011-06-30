/* Hyperdatons generated from AST::Expr.
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

/**
 * @file compiled_functors.cpp
 * The WSs that implement the evaluation of expressions.
 */

#include <tl/builtin_types.hpp>
#include <tl/compiled_functors.hpp>
#include <tl/consthd.hpp>
#include <tl/fixed_indexes.hpp>
#include <tl/maxsharelist.hpp>
#include <tl/system.hpp>
#include <tl/types/dimension.hpp>
#include <tl/types/function.hpp>
#include <tl/types/special.hpp>
#include <tl/types/tuple.hpp>
#include <tl/types_util.hpp>
#include <tl/utility.hpp>

#include <sstream>

//TODO: work out what looking up an identifier is
//TODO: what do we do with VariableOpWS

namespace TransLucid
{

namespace Hyperdatons
{

TaggedConstant
BoolConstWS::operator()(const Tuple& k)
{
  //return TaggedConstant(Constant(Boolean(m_value), TYPE_INDEX_BOOL), k);
  return TaggedConstant(m_value, k);
}

TaggedConstant
TypeConstWS::operator()(const Tuple& k)
{
  return TaggedConstant(m_value, k);
}

TaggedConstant
DimensionWS::operator()(const Tuple& k)
{
  return TaggedConstant (Types::Dimension::create(m_id), k);
}

TaggedConstant
IdentWS::operator()(const Tuple& k)
{
  WS* e = m_identifiers.lookup(m_name);

  if (e != 0)
  {
    return (*e)(k);
  }
  else
  {
    return TaggedConstant(Types::Special::create(SP_UNDEF), k);
  }
}

TaggedConstant
BangOpWS::operator()(const Tuple& k)
{
  //lookup function in the system and call it

  //evaluate name expr
  Constant name = (*m_name)(k).first;

  if (name.index() != TYPE_INDEX_USTRING)
  {
    return TaggedConstant(Types::Special::create(SP_UNDEF), k);
  }
  else
  {
    //evaluate all the args in context k and pass them as the parameters
    //to the function
    return m_caller(get_constant_pointer<u32string>(name), m_args, k);
  }
}

TaggedConstant
IfWS::operator()(const Tuple& k)
{
  TaggedConstant cond = (*m_condition)(k);
  Constant& condv = cond.first;

  if (condv.index() == TYPE_INDEX_SPECIAL)
  {
    return cond;
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
      std::list<WS*>::const_iterator iter = m_elsifs.begin();
      while (iter != m_elsifs.end())
      {
        //std::auto_ptr<ValueV> cond(makeValue((*iter)->visit(this, d)));
        TaggedConstant cond = (*iter)->operator()(k);

        type_index index = cond.first.index();

        if (index == TYPE_INDEX_SPECIAL)
        {
          return cond;
        }
        else if (index == TYPE_INDEX_BOOL)
        {
          bool bcond = get_constant<bool>(cond.first);
          ++iter;
          if (bcond)
          {
            return (*iter)->operator()(k);
          }
        }
        else
        {
          return TaggedConstant(Types::Special::create(SP_TYPEERROR), k);
        }

        ++iter;
      }

      return (*m_else)(k);
    }
  }
  else
  {
    return TaggedConstant(Types::Special::create(SP_TYPEERROR),k);
  }
}

TaggedConstant
HashWS::operator()(const Tuple& k)
{
  TaggedConstant r = (*m_e)(k);
  return lookup_context(m_system, r.first, k);
}

TaggedConstant
IntmpConstWS::operator()(const Tuple& k)
{
  return TaggedConstant(m_value, k);
}

TaggedConstant
IsSpecialWS::operator()(const Tuple& k)
{
  //this is going away, but to stop warnings
  return TaggedConstant();
}

TaggedConstant
IsTypeWS::operator()(const Tuple& k)
{
  //this is going away, but to stop warnings
  return TaggedConstant();
}

TaggedConstant
SpecialConstWS::operator()(const Tuple& k)
{
  return TaggedConstant(Constant(Special(m_value), TYPE_INDEX_SPECIAL), k);
}

TaggedConstant
UStringConstWS::operator()(const Tuple& k)
{
  return TaggedConstant(m_value, k);
}

TaggedConstant
UCharConstWS::operator()(const Tuple& k)
{
  //return TaggedConstant(Constant(Char(m_value), TYPE_INDEX_UCHAR), k);
  return TaggedConstant(m_value, k);
}

TaggedConstant
UnaryOpWS::operator()(const Tuple& k)
{
  //TODO: resolve what to do with operators
  return TaggedConstant();
}

TaggedConstant
TupleWS::operator()(const Tuple& k)
{
  tuple_t kp;
  for(auto& pair : m_elements)
  {
    //const Pair& p = v.first.value<Pair>();
    TaggedConstant left = (*pair.first)(k);
    TaggedConstant right = (*pair.second)(k);

    if (left.first.index() == TYPE_INDEX_DIMENSION)
    {
      kp[get_constant<dimension_index>(left.first)] = right.first;
    }
    else
    {
      kp[m_system.getDimensionIndex(left.first)] = right.first;
    }
  }
  return TaggedConstant(Types::Tuple::create(Tuple(kp)), k);
}

TaggedConstant
AtWS::operator()(const Tuple& k)
{
  tuple_t kNew = k.tuple();
  TaggedConstant kp = (*e1)(k);
  if (kp.first.index() != TYPE_INDEX_TUPLE)
  {
    return TaggedConstant(Types::Special::create(SP_TYPEERROR), k);
  }
  else
  {
    //validate time
    auto& delta = Types::Tuple::get(kp.first).tuple();
    const auto& dimTime = delta.find(DIM_TIME);
    if (dimTime != delta.end() && Types::Intmp::get(dimTime->second) > 
      Types::Intmp::get(kNew[DIM_TIME]))
    {
      return TaggedConstant(Types::Special::create(SP_ACCESS), k);
    }

    for(tuple_t::value_type v : delta)
    {
      kNew[v.first] = v.second;
    }
    return (*e2)(Tuple(kNew));
  }
}

TaggedConstant
LambdaAbstractionWS::operator()(const Tuple& k)
{
  return TaggedConstant(
    Types::Function::create(LambdaFunctionType(m_name, m_dim, m_rhs)),
    k
  );
}

TaggedConstant
LambdaApplicationWS::operator()(const Tuple& k)
{
  //evaluate the lhs, evaluate the rhs
  //and pass the value to the function to evaluate

  Constant lhs = (*m_lhs)(k).first;
  //first make sure that it is a function
  if (lhs.index() != TYPE_INDEX_FUNCTION)
  {
    return TaggedConstant(Types::Special::create(SP_TYPEERROR), k);
  }

  Constant rhs = (*m_rhs)(k).first;
  const FunctionType& f = Types::Function::get(lhs);

  return f.applyLambda(k, rhs);
}

} //namespace Hyperdatons

} //namespace TransLucid
