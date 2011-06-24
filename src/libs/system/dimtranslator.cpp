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
#include <boost/assign/list_of.hpp>
#include <tl/fixed_indexes.hpp>

#include <initializer_list>

namespace TransLucid
{

DimensionTranslator::DimensionTranslator()
: m_nextIndex(DIM_INDEX_LAST)
, m_named(m_nextIndex, std::vector<std::pair<u32string, dimension_index>>
  {
   {U"type", DIM_TYPE},
   {U"text", DIM_TEXT},
   {U"name", DIM_NAME},
   {U"id", DIM_ID},
   {U"value", DIM_VALUE},
   {U"time", DIM_TIME},
   {U"all", DIM_ALL}
  }
  )
, m_constants(m_nextIndex)
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
  return m_constants(value);
}

}
