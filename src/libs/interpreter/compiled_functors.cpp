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
  k[get_dimension_index(h, dim)] = TypedValue(T(value), index);
}

inline void
contextInsert
(
  tuple_t& k,
  HD* i,
  const u32string& dim,
  const TypedValue& v
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
    m_k[get_dimension_index(m_h, dim)] = TypedValue(T(v), m_index);
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

TaggedValue
BinaryOpHD::operator()(const Tuple& k)
{
  //TypedValue v1 = (*m_operands.at(0))(k).first;
  //TypedValue v2 = (*m_operands.at(1))(k).first;
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

  TaggedValue v = (*m_system)(Tuple(t));
  //std::cerr << "result: " << v.first << std::endl;
  return TaggedValue(v.first, k);
}

TaggedValue
BoolHD::operator()(const Tuple& k)
{
  return TaggedValue(TypedValue(Boolean(m_value), TYPE_INDEX_BOOL), k);
}

TaggedValue
TypedValueHD::operator()(const Tuple& k)
{
  //evaluate CONST
  tuple_t kp = k.tuple();
  //kp[m_system.dimTranslator().lookup("id")] =
  //  TypedValue(String("CONST"), m_system.typeRegistry().indexString());
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

TaggedValue
Convert::operator()(const Tuple& context)
{
  //going away
  return TaggedValue();
}

TaggedValue
DimensionHD::operator()(const Tuple& k)
{
  size_t id = get_dimension_index(m_system, m_name);
  return TaggedValue (TypedValue(TransLucid::Dimension(id),
                      TYPE_INDEX_DIMENSION), k);
}

TaggedValue
IdentHD::operator()(const Tuple& k)
{
  tuple_t kp = k.tuple();

  insert_tuple<String>
    (kp, m_system, TYPE_INDEX_USTRING)
    (U"id", m_name)
    ;
  return (*m_system)(Tuple(kp));
}

TaggedValue
IfHD::operator()(const Tuple& k)
{
  TaggedValue cond = (*m_condition)(k);
  TypedValue& condv = cond.first;

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
        TaggedValue cond = (*iter)->operator()(k);

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
          return TaggedValue(TypedValue(Special(Special::TYPEERROR),
                             TYPE_INDEX_SPECIAL), k);
        }

        ++iter;
      }

      return (*m_else)(k);
    }
  }
  else
  {
    return TaggedValue(TypedValue(Special(Special::TYPEERROR),
                       TYPE_INDEX_SPECIAL), k);
  }
}

TaggedValue
HashHD::operator()(const Tuple& k)
{
  TaggedValue r = (*m_e)(k);
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
    return TaggedValue(iter->second, k);
  }
  else
  {
    return TaggedValue(TypedValue(Special(Special::DIMENSION),
                       TYPE_INDEX_SPECIAL), k);
  }
}

TaggedValue
IntegerConstHD::operator()(const Tuple& k)
{
  return TaggedValue(TypedValue(Intmp(m_value), TYPE_INDEX_INTMP), k);
}

TaggedValue
IsSpecial::operator()(const Tuple& context)
{
  //this is going away, but to stop warnings
  return TaggedValue();
}

TaggedValue
IsType::operator()(const Tuple& context)
{
  //this is going away, but to stop warnings
  return TaggedValue();
}

TaggedValue
OperationHD::operator()(const Tuple& k)
{
  tuple_t kp = k.tuple();

  TupleInserter<String> insert(kp, m_system, TYPE_INDEX_USTRING);
  insert(U"id", U"FUN");

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
TaggedValue
Pair::operator()(const Tuple& k)
{
  TaggedValue l = (*m_lhs)(k);
  TaggedValue r = (*m_rhs)(k);

  return TaggedValue(TypedValue(PairType((*m_lhs)(k).first, (*m_rhs)(k).first),
                     TYPE_INDEX_PAIR), k);
}
#endif

TaggedValue
SpecialHD::operator()(const Tuple& k)
{
  return TaggedValue(TypedValue(Special(m_value), TYPE_INDEX_SPECIAL), k);
}

TaggedValue
StringConstHD::operator()(const Tuple& k)
{
  return TaggedValue(TypedValue(String(m_value), TYPE_INDEX_USTRING), k);
}

TaggedValue
UcharConstHD::operator()(const Tuple& k)
{
  return TaggedValue(TypedValue(Char(m_value), TYPE_INDEX_UCHAR), k);
}

TaggedValue
UnaryOpHD::operator()(const Tuple& context)
{
  //TODO: resolve what to do with operations
  return TaggedValue();
}

TaggedValue
BuildTupleHD::operator()(const Tuple& k)
{
  tuple_t kp;
  BOOST_FOREACH(auto& pair, m_elements)
  {
    //const PairType& p = v.first.value<PairType>();
    TaggedValue left = (*pair.first)(k);
    TaggedValue right = (*pair.second)(k);

    if (left.first.index() == TYPE_INDEX_DIMENSION)
    {
      kp[left.first.value<TransLucid::Dimension>().value()] = right.first;
    }
    else
    {
      kp[get_dimension_index(m_system, left.first)] = right.first;
    }
  }
  return TaggedValue(TypedValue(Tuple(kp), TYPE_INDEX_TUPLE), k);
}

TaggedValue
AtAbsoluteHD::operator()(const Tuple& k)
{
  TaggedValue kp = (*e1)(k);
  if (kp.first.index() != TYPE_INDEX_TUPLE)
  {
    return TaggedValue(TypedValue(Special(Special::TYPEERROR),
      TYPE_INDEX_SPECIAL), k);
  }
  else
  {
    return (*e2)(kp.first.value<Tuple>());
  }
}

TaggedValue
AtRelativeHD::operator()(const Tuple& k)
{
  tuple_t kNew = k.tuple();
  TaggedValue kp = (*e1)(k);
  if (kp.first.index() != TYPE_INDEX_TUPLE)
  {
    return TaggedValue(TypedValue(Special(Special::TYPEERROR),
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
