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
#include <tl/utility.hpp>

#include <sstream>

//TODO: work out what looking up an identifier is
//TODO: what do we do with VariableOpWS

namespace TransLucid
{

namespace Hyperdatons
{

namespace
{

#if 0
template <typename T>
class TupleInserter
{
  public:
  TupleInserter(tuple_t& k, WS* h, size_t index)
  : m_k(k), m_h(h), m_index(index)
  {
  }

  template <typename V>
  const TupleInserter<T>&
  operator()(const u32string& dim, const V& v) const
  {
    m_k[get_dimension_index(m_h, dim)] = Constant(T(v), m_index);
    return *this;
  }

  private:
  tuple_t& m_k;
  WS* m_h;
  size_t m_index;
};

template <typename T>
TupleInserter<T>
insert_tuple(tuple_t& k, WS* h, size_t index)
{
  return TupleInserter<T>(k, h, index);
}
#endif

}

//don't need this one because we are doing it properly with eval
#if 0
TaggedConstant
SystemEvaluationWS::operator()(const Tuple& k)
{
  tuple_t newK = k.tuple();
  newK[DIM_TIME] = Constant(Intmp(m_system->theTime()), TYPE_INDEX_INTMP);
  return (*m_e)(Tuple(newK));
}
#endif

//this one is compiled out now
#if 0
TaggedConstant
BinaryOpWS::operator()(const Tuple& k)
{
  //Constant v1 = (*m_operands.at(0))(k).first;
  //Constant v2 = (*m_operands.at(1))(k).first;
  //std::cerr << "operands to binary op " << m_name << ":" << std::endl;
  //std::cerr << v1 << std::endl;
  //std::cerr << v2 << std::endl;
  tuple_t t =
  {
    {get_dimension_index(m_system, U"arg0"), (*m_operands.at(0))(k).first},
    {get_dimension_index(m_system, U"arg1"), (*m_operands.at(1))(k).first},
    {DIM_ID, generate_string(U"OP")},
    {DIM_NAME, generate_string(m_name)}
  };

  tuple_t kNew = k.tuple();
  kNew.insert(t.begin(), t.end());

  //std::cerr << "finding OP @ [name : " << utf32_to_utf8(m_name) << " ..." << std::endl;

  TaggedConstant v = (*m_system)(Tuple(kNew));
  //std::cerr << "result: " << v.first << std::endl;
  return TaggedConstant(v.first, k);
}
#endif

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


#if 0
TaggedConstant
TypedValueWS::operator()(const Tuple& k)
{
  //evaluate CONST
  tuple_t kp = k.tuple();

  insert_tuple<String>
    (kp, m_system, TYPE_INDEX_USTRING)
    (U"id", U"CONST")
    (U"type", m_type)
    (U"text", m_text)
  ;

  return (*m_system)(Tuple(kp));
}
#endif

//TODO work out type conversion
#if 0
TaggedConstant
ConvertWS::operator()(const Tuple& k)
{
  return TaggedConstant();
}
#endif

TaggedConstant
DimensionWS::operator()(const Tuple& k)
{
  dimension_index id = m_system.getDimensionIndex(m_name);
  return TaggedConstant (Types::Dimension::create(id), k);
}

TaggedConstant
IdentWS::operator()(const Tuple& k)
{
  #if 0
  tuple_t kp = k.tuple();

  tuple_t::iterator itid = kp.find(DIM_ID);
  if (itid != kp.end())
  {
    itid->second = Constant(
      String(m_name + U"." + itid->second.value<String>().value()),
      TYPE_INDEX_USTRING
     );
  }
  else 
  {
    insert_tuple<String>
      (kp, m_system, TYPE_INDEX_USTRING)
      (U"id", m_name)
      ;
  }
  return (*m_system)(Tuple(kp));
  #endif
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
  //std::cerr << "hash " << r.first << " = ";
  #if 0
  size_t index;
  if (r.first.index() == TYPE_INDEX_DIMENSION)
  {
    index = r.first.value<TransLucid::Dimension>().value();
  }
  else
  {
    index = get_dimension_index(m_system, r.first);
  }

  Tuple::const_iterator iter = k.find(index);
  if (iter != k.end())
  {
    //std::cerr << iter->second << std::endl;
    return TaggedConstant(iter->second, k);
  }
  else
  {
    //find the all dimension
    Tuple::const_iterator all = k.find(DIM_ALL);
    if (all == k.end())
    {
      return TaggedConstant(Constant(Special(Special::DIMENSION),
                            TYPE_INDEX_SPECIAL), k);
    }
    else
    {
      return TaggedConstant(all->second, k);
    }
  }
  #endif
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

//I don't think we even use this
#if 0
TaggedConstant
VariableOpWS::operator()(const Tuple& k)
{
  tuple_t kp = k.tuple();

  TupleInserter<String> insert(kp, m_system, TYPE_INDEX_USTRING);
  insert(U"id", U"OP");

  int i = 0;
  std::ostringstream os;
  for(WS* h : m_operands)
  {
    os.str("arg");
    os << i;
    kp[get_dimension_index(h, to_u32string(os.str()))] = (*h)(k).first;
    ++i;
  }

  return (*m_system)(Tuple(kp));
}
#endif

#if 0
TaggedConstant
PairWS::operator()(const Tuple& k)
{
  TaggedConstant l = (*m_lhs)(k);
  TaggedConstant r = (*m_rhs)(k);

  return TaggedConstant(Constant(Pair((*m_lhs)(k).first, (*m_rhs)(k).first),
                     TYPE_INDEX_PAIR), k);
}
#endif

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
      return TaggedConstant(make_special(SP_ACCESS), k);
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
    return TaggedConstant(make_special(SP_TYPEERROR), k);
  }

  Constant rhs = (*m_rhs)(k).first;
  const FunctionType& f = Types::Function::get(lhs);

  return f.applyLambda(k, rhs);
}

} //namespace Hyperdatons

} //namespace TransLucid
