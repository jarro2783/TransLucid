/* The "true" constant hyperdatons.
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

#include <tl/valuehd.hpp>
#include <tl/builtin_types.hpp>
#include <tl/utility.hpp>

namespace TransLucid
{

namespace Hyperdatons
{

const char32_t* BoolHD::name =      U"bool";
const char32_t* IntmpHD::name =     U"intmp";
const char32_t* UCharHD::name =     U"uchar";
const char32_t* UStringHD::name =   U"ustring";

TaggedConstant
BoolHD::operator()(const Tuple& k)
{
  Tuple::const_iterator value = k.find(DIM_TEXT);

  if (value == k.end() || value->second.index() != TYPE_INDEX_USTRING)
  {
    return TaggedConstant(Constant(Special(Special::DIMENSION),
                          TYPE_INDEX_SPECIAL), k);
  }

  if (value->second.value<String>().value() == U"true")
    return TaggedConstant(Constant(Boolean(true),
                          TYPE_INDEX_BOOL), k);
  else if (value->second.value<String>().value() == U"false")
    return TaggedConstant(Constant(Boolean(false),
                          TYPE_INDEX_BOOL), k);
  else
    return TaggedConstant(Constant(Special(Special::CONST),
                          TYPE_INDEX_SPECIAL), k);
}

TaggedConstant
IntmpHD::operator()(const Tuple& k)
{
  Tuple::const_iterator value = k.find(DIM_TEXT);

  if (value == k.end() || value->second.index() != TYPE_INDEX_USTRING)
  {
    return TaggedConstant(Constant(Special(Special::DIMENSION),
                          TYPE_INDEX_SPECIAL), k);
  }

  try
  {
    return TaggedConstant(Constant(TransLucid::Intmp(
             mpz_class(u32_to_ascii(value->second.value<String>().value()))),
             TYPE_INDEX_INTMP), k);
  }
  catch (...)
  {
    return TaggedConstant(Constant(Special(Special::CONST),
                          TYPE_INDEX_SPECIAL), k);
  }
}

TaggedConstant
UCharHD::operator()(const Tuple& k)
{
  size_t valueindex = get_dimension_index(m_system, U"text");
  Tuple::const_iterator value = k.find(valueindex);

  if (value == k.end() || value->second.index() != TYPE_INDEX_USTRING)
  {
    return TaggedConstant(Constant(Special(Special::DIMENSION),
                          TYPE_INDEX_SPECIAL), k);
  }

  const u32string& s = value->second.value<String>().value();
  //return TaggedConstant(m_i.typeRegistry().findType("uchar")
  //         ->parse(s.value(), k, m_i), k);
  if (s.length() != 1)
  {
    return TaggedConstant(Constant(Special(Special::CONST),
                          TYPE_INDEX_SPECIAL), k);
  }
  return TaggedConstant(Constant(Char(s[0]), TYPE_INDEX_UCHAR), k);
}

TaggedConstant
UStringHD::operator()(const Tuple& k)
{
  Tuple::const_iterator value = k.find(DIM_TEXT);

  if (value == k.end() || value->second.index() != TYPE_INDEX_USTRING)
  {
    return TaggedConstant(Constant(Special(Special::DIMENSION),
                          TYPE_INDEX_SPECIAL), k);
  }
  return TaggedConstant(value->second, k);
}

} //namespace Hyperdatons

} //namespace TransLucid
