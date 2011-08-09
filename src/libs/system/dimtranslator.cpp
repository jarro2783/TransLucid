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

#include <tl/dimtranslator.hpp>
#include <tl/fixed_indexes.hpp>
#include <tl/types/intmp.hpp>

#include <initializer_list>

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
   {U"time", DIM_TIME},
   {U"all", DIM_ALL}
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

size_t
DimensionTranslator::lookup(const u32string& name)
{
  return m_named(name);
}

size_t
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

}
