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

#ifndef TL_CACHE_HPP_INCLUDED
#define TL_CACHE_HPP_INCLUDED

#include <map>
#include <tl/context.hpp>
#include <tl/types.hpp>
#include <tl/variant.hpp>

namespace TransLucid
{
  class CacheLevel;
  
  typedef Variant
  <
    Constant,
    recursive_wrapper<CacheLevel>
  > CacheEntry;

  struct CacheEntryMap;

  typedef Variant
  <
    recursive_wrapper<CacheEntryMap>,
    CacheEntry
  > CacheLevelNode;

  struct CacheEntryMap
  {
    std::map<Constant, CacheLevelNode> entry;
  };

  struct CacheLevel
  {
    std::vector<dimension_index> dims;
    CacheLevelNode entry;
  };
  
  class Cache
  {
    public:

    Cache() = default;
    
    Constant
    get(Context& delta) const;

    void
    set(const Context& delta, const Constant& value);

    private:
    CacheEntry m_entry;
  };
}

#endif // CACHE_HPP_INCLUDED
