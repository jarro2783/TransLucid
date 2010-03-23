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

#ifndef CACHE_HPP_INCLUDED
#define CACHE_HPP_INCLUDED

#include <map>
#include <tl/types.hpp>

namespace TransLucid
{
  class Interpreter;

  class LazyWarehouse
  {
    public:

    LazyWarehouse(Interpreter& i)
    : m_interpreter(i)
    {}

    //looks up and if not found adds a calc entry
    std::pair<bool, TypedValue>
    lookupCalc(const u32string& name, const Tuple& c);

    void
    add(const u32string& name, const TypedValue& value, const Tuple& c);

    private:
    typedef std::map<Tuple, TypedValue> TupleToValue;
    typedef std::map<u32string, TupleToValue> CacheMapping;
    CacheMapping m_cache;
    Interpreter& m_interpreter;
  };

}

#endif // CACHE_HPP_INCLUDED
