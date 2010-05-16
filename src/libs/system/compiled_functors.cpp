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

#include <tl/compiled_functors.hpp>
#include <boost/assign/list_of.hpp>
#include <tl/builtin_types.hpp>
#include <tl/fixed_indexes.hpp>
#include <tl/utility.hpp>

namespace TransLucid
{

namespace Hyperdatons
{

namespace
{

template <typename T, typename V>
inline void
contextInsert
(
  tuple_t& k,
  HD* h,
  const u32string& dim,
  const V& value,
  size_t index
)
{
  k[get_dimension_index(h, dim)] = Constant(T(value), index);
}

inline void
contextInsert
(
  tuple_t& k,
  HD* i,
  const u32string& dim,
  const Constant& v
)
{
  k[get_dimension_index(i, dim)] = v;
}

template <typename T>
class TupleInserter
{
  public:
  TupleInserter(tuple_t& k, HD* h, size_t index)
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
  HD* m_h;
  size_t m_index;
};

template <typename T>
TupleInserter<T>
insert_tuple(tuple_t& k, HD* h, size_t index)
{
  return TupleInserter<T>(k, h, index);
}

}

TaggedConstant
BinaryOpHD::operator()(const Tuple& k)
{
  //Constant v1 = (*m_operands.at(0))(k).first;
  //Constant v2 = (*m_operands.at(1))(k).first;
  //std::cerr << "operands to binary op " << m_name << ":" << std::endl;
  //std::cerr << v1 << std::endl;
  //std::cerr << v2 << std::endl;
  #warning at the moment hack it for binary, variadic will have to wait
  tuple_t t =
  {
    {get_dimension_index(m_system, U"arg0"), (*m_operands.at(0))(k).first},
    {get_dimension_index(m_system, U"arg1"), (*m_operands.at(1))(k).first},
    {DIM_ID, generate_string(U"OP")},
    {DIM_NAME, generate_string(m_name)}
  };

  //std::cerr << "finding OP @ [name : " << utf32_to_utf8(m_name) << " ..." << std::endl;

  TaggedConstant v = (*m_system)(Tuple(t));
  //std::cerr << "result: " << v.first << std::endl;
  return TaggedConstant(v.first, k);
}

TaggedConstant
BoolHD::operator()(const Tuple& k)
{
  return TaggedConstant(Constant(Boolean(m_value), TYPE_INDEX_BOOL), k);
}

TaggedConstant
TypedValueHD::operator()(const Tuple& k)
{
  //evaluate CONST
  tuple_t kp = k.tuple();
  //kp[m_system.dimTranslator().lookup("id")] =
  //  Constant(String("CONST"), m_system.typeRegistry().indexString());
  //contextInsertString(kp, m_system, "id", "CONST", indexString);
  //contextInsertString(kp, m_system, "type", m_type, indexString);
  //contextInsertString(kp, m_system, "text", m_text, indexString);
  insert_tuple<String>
    (kp, m_system, TYPE_INDEX_USTRING)
    (U"id", U"CONST")
    (U"type", m_type)
    (U"text", m_text)
  ;

  return (*m_system)(Tuple(kp));
}

TaggedConstant
ConvertHD::operator()(const Tuple& k)
{
  //going away
  return TaggedConstant();
}

TaggedConstant
DimensionHD::operator()(const Tuple& k)
{
  size_t id = get_dimension_index(m_system, m_name);
  return TaggedConstant (Constant(TransLucid::Dimension(id),
                      TYPE_INDEX_DIMENSION), k);
}

TaggedConstant
IdentHD::operator()(const Tuple& k)
{
  tuple_t kp = k.tuple();

  insert_tuple<String>
    (kp, m_system, TYPE_INDEX_USTRING)
    (U"id", m_name)
    ;
  return (*m_system)(Tuple(kp));
}

TaggedConstant
IfHD::operator()(const Tuple& k)
{
  TaggedConstant cond = (*m_condition)(k);
  Constant& condv = cond.first;

  if (condv.index() == TYPE_INDEX_SPECIAL)
  {
    return cond;
  }
  else if (condv.index() == TYPE_INDEX_BOOL)
  {
    const Boolean& b = condv.value<Boolean>();

    if (b)
    {
      return (*m_then)(k);
      //result = makeValue(e->then->visit(this, d));
    }
    else
    {
      //run the elsifs and else
      std::list<HD*>::const_iterator iter = m_elsifs.begin();
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
          const Boolean& bcond = cond.first.value<Boolean>();
          ++iter;
          if (bcond)
          {
            return (*iter)->operator()(k);
          }
        }
        else
        {
          return TaggedConstant(Constant(Special(Special::TYPEERROR),
                             TYPE_INDEX_SPECIAL), k);
        }

        ++iter;
      }

      return (*m_else)(k);
    }
  }
  else
  {
    return TaggedConstant(Constant(Special(Special::TYPEERROR),
                       TYPE_INDEX_SPECIAL), k);
  }
}

TaggedConstant
HashHD::operator()(const Tuple& k)
{
  TaggedConstant r = (*m_e)(k);
  //std::cerr << "hash " << r.first << " = ";
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
    return TaggedConstant(Constant(Special(Special::DIMENSION),
                       TYPE_INDEX_SPECIAL), k);
  }
}

TaggedConstant
IntegerConstHD::operator()(const Tuple& k)
{
  return TaggedConstant(Constant(Intmp(m_value), TYPE_INDEX_INTMP), k);
}

TaggedConstant
IsSpecialHD::operator()(const Tuple& k)
{
  //this is going away, but to stop warnings
  return TaggedConstant();
}

TaggedConstant
IsTypeHD::operator()(const Tuple& k)
{
  //this is going away, but to stop warnings
  return TaggedConstant();
}

TaggedConstant
OperationHD::operator()(const Tuple& k)
{
  tuple_t kp = k.tuple();

  TupleInserter<String> insert(kp, m_system, TYPE_INDEX_USTRING);
  insert(U"id", U"OP");

  int i = 0;
  std::ostringstream os;
  BOOST_FOREACH(HD* h, m_operands)
  {
    os.str("arg");
    os << i;
    kp[get_dimension_index(h, to_u32string(os.str()))] = (*h)(k).first;
    ++i;
  }

  return (*m_system)(Tuple(kp));
}

#if 0
TaggedConstant
Pair::operator()(const Tuple& k)
{
  TaggedConstant l = (*m_lhs)(k);
  TaggedConstant r = (*m_rhs)(k);

  return TaggedConstant(Constant(PairType((*m_lhs)(k).first, (*m_rhs)(k).first),
                     TYPE_INDEX_PAIR), k);
}
#endif

TaggedConstant
SpecialHD::operator()(const Tuple& k)
{
  return TaggedConstant(Constant(Special(m_value), TYPE_INDEX_SPECIAL), k);
}

TaggedConstant
StringConstHD::operator()(const Tuple& k)
{
  return TaggedConstant(Constant(String(m_value), TYPE_INDEX_USTRING), k);
}

TaggedConstant
UcharConstHD::operator()(const Tuple& k)
{
  return TaggedConstant(Constant(Char(m_value), TYPE_INDEX_UCHAR), k);
}

TaggedConstant
UnaryOpHD::operator()(const Tuple& k)
{
  //TODO: resolve what to do with operations
  return TaggedConstant();
}

TaggedConstant
TupleHD::operator()(const Tuple& k)
{
  tuple_t kp;
  BOOST_FOREACH(auto& pair, m_elements)
  {
    //const PairType& p = v.first.value<PairType>();
    TaggedConstant left = (*pair.first)(k);
    TaggedConstant right = (*pair.second)(k);

    if (left.first.index() == TYPE_INDEX_DIMENSION)
    {
      kp[left.first.value<TransLucid::Dimension>().value()] = right.first;
    }
    else
    {
      kp[get_dimension_index(m_system, left.first)] = right.first;
    }
  }
  return TaggedConstant(Constant(Tuple(kp), TYPE_INDEX_TUPLE), k);
}

TaggedConstant
AtAbsoluteHD::operator()(const Tuple& k)
{
  TaggedConstant kp = (*e1)(k);
  if (kp.first.index() != TYPE_INDEX_TUPLE)
  {
    return TaggedConstant(Constant(Special(Special::TYPEERROR),
      TYPE_INDEX_SPECIAL), k);
  }
  else
  {
    return (*e2)(kp.first.value<Tuple>());
  }
}

TaggedConstant
AtRelativeHD::operator()(const Tuple& k)
{
  tuple_t kNew = k.tuple();
  TaggedConstant kp = (*e1)(k);
  if (kp.first.index() != TYPE_INDEX_TUPLE)
  {
    return TaggedConstant(Constant(Special(Special::TYPEERROR),
      TYPE_INDEX_SPECIAL), k);
  }
  else
  {
    BOOST_FOREACH(tuple_t::value_type v, kp.first.value<Tuple>().tuple())
    {
      kNew[v.first] = v.second;
    }
    return (*e2)(Tuple(kNew));
  }
}

} //namespace Hyperdatons

} //namespace TransLucid
