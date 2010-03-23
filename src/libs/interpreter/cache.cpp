/* The warehouse.
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

#include <tl/cache.hpp>
#include <tl/types.hpp>
#include <tl/builtin_types.hpp>
#include <tl/interpreter.hpp>

namespace TransLucid
{

std::pair<bool, TypedValue>
LazyWarehouse::lookupCalc(const u32string& name, const Tuple& c)
{
  #if 0
  CacheMapping::iterator iter = m_cache.find(name);
  if (iter == m_cache.end())
  {
    //add calc because it wasn't found
    TupleToValue m;
    m.insert(std::make_pair
      (c, TypedValue(ValueCalc(), m_interpreter.typeRegistry().indexCalc())));
    m_cache.insert(std::make_pair(name, m));
  }
  else
  {
    TupleToValue& values = iter->second;
    TupleToValue::iterator titer = values.find(c);
    if (titer == values.end())
    {
      //add calc because it wasn't found
      iter->second.insert
        (std::make_pair
          (c, TypedValue(ValueCalc(),
                         m_interpreter.typeRegistry().indexCalc())));
    }
    else
    {
      //return the actual value
      return std::make_pair(true, titer->second);
    }
  }
  #endif
  return std::make_pair(false, TypedValue());
}

void
LazyWarehouse::add
(const u32string& name, const TypedValue& value, const Tuple& c)
{
  #if 0
  CacheMapping::iterator iter = m_cache.find(name);
  if (iter == m_cache.end())
  {
    TupleToValue m;
    m.insert(std::make_pair(c, value));
    m_cache.insert(std::make_pair(name, m));
  }
  else
  {
    TupleToValue::iterator titer = iter->second.find(c);
    if (titer == iter->second.end())
    {
      iter->second.insert(std::make_pair(c, value));
    }
    else
    {
      titer->second = value;
    }
  }
  #endif
}

}
