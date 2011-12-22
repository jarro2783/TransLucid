/* Maps dimensions to indexes.
   Copyright (C) 2009, 2010, 2011 Jarryd Beck and John Plaice

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

#include <vector>

#include <tl/dimtranslator.hpp>
#include <tl/fixed_indexes.hpp>
#include <tl/types/intmp.hpp>

namespace TransLucid
{

DimensionTranslator::DimensionTranslator()
: m_nextIndex(DIM_INDEX_LAST)
, m_named(m_nextIndex, std::vector<std::pair<u32string, dimension_index>>
  {
   {U"typename", DIM_TYPE},
   {U"text", DIM_TEXT},
   {U"name", DIM_NAME},
   {U"id", DIM_ID},
   {U"value", DIM_VALUE},
   {U"symbol", DIM_SYMBOL},
   {U"time", DIM_TIME},
   {U"all", DIM_ALL},
   {U"cons", DIM_CONS},
   {U"type", DIM_TYPE},
   {U"arg0", DIM_ARG0},
   {U"arg1", DIM_ARG1},
   {U"arg2", DIM_ARG2}
  }
  )
, m_constants(m_nextIndex, std::vector<std::pair<Constant, dimension_index>>
  {
    {Types::Intmp::create(0), DIM_ZERO},
    {Types::Intmp::create(1), DIM_ONE},
    {Types::Intmp::create(2), DIM_TWO}
  }
  )
{
}

dimension_index
DimensionTranslator::lookup(const u32string& name)
{
  return m_named(name);
}

dimension_index
DimensionTranslator::lookup(const Constant& value)
{
  if (value.index() == TYPE_INDEX_DIMENSION)
  {
    return get_constant<dimension_index>(value);
  }
  else
  {
    return m_constants(value);
  }
}

const u32string*
DimensionTranslator::reverse_lookup_named(dimension_index dim) const
{
  return m_named.reverseLookup(dim);
}

const Constant*
DimensionTranslator::reverse_lookup_constant(dimension_index dim) const
{
  return m_constants.reverseLookup(dim);
}

}
