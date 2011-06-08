/* Maps dimensions to indexes.
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

#include <tl/dimtranslator.hpp>
#include <boost/assign/list_of.hpp>
#include <tl/fixed_indexes.hpp>

namespace TransLucid
{

DimensionTranslator::DimensionTranslator()
: m_nextIndex(DIM_INDEX_LAST),
  m_namedDims
  {
   {U"type", DIM_TYPE},
   {U"text", DIM_TEXT},
   {U"name", DIM_NAME},
   {U"id", DIM_ID},
   {U"value", DIM_VALUE},
   {U"time", DIM_TIME},
   {U"_validguard", DIM_VALID_GUARD},
   {U"all", DIM_ALL}
  }
{
}

size_t
DimensionTranslator::lookup(const u32string& name)
{
  std::pair<ustring_size_map::iterator,bool> result =
    m_namedDims.insert(std::make_pair(name, m_nextIndex));
  if (result.second)
  {
    ++m_nextIndex;
  }
  return result.first->second;
}

size_t
DimensionTranslator::lookup(const Constant& value)
{
  std::pair<ustring_type_map::iterator,bool> result =
    m_typedDims.insert(std::make_pair(value, m_nextIndex));
  if (result.second)
  {
    ++m_nextIndex;
  }

  return result.first->second;
}

}
