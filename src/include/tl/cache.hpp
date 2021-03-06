/* Cache header.
   Copyright (C) 2009--2013 Jarryd Beck

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
#include <set>

#include <tl/context.hpp>
#include <tl/types.hpp>
#include <tl/variant.hpp>
#include <tl/workshop.hpp>

namespace TransLucid
{
  //an actual cache entry, either a value or a single level in the cache
  //hierarchy
  struct CacheEntry;

  //one level in the hierarchy, the relevant dimensions and
  struct CacheLevel;
  struct CacheEntryMap;
  struct CacheLevelNode;
  
  typedef Variant
  <
    Constant,
    recursive_wrapper<CacheLevel>
  > CacheEntryVariant;

  typedef Variant
  <
    recursive_wrapper<CacheEntryMap>,
    CacheEntry
  > CacheLevelNodeVariant;

  struct CacheEntry
  {
    CacheEntry() = default;

    CacheEntry(CacheEntryVariant&& v)
    : entry(std::move(v)), age(0)
    {
    }

    CacheEntryVariant entry;

    int age;
  };

  struct CacheLevelNode
  {
    CacheLevelNode(CacheLevelNodeVariant&& v)
    : entry(std::move(v))
    {
    }

    CacheLevelNodeVariant entry;
  };

  struct CacheEntryMap
  {
    std::map<Constant, CacheLevelNode> entry;
  };

  struct CacheLevel
  {
    //a cache level with required dimensions, entry must be a map
    CacheLevel(const std::set<dimension_index>& dimset)
    : dims(dimset.begin(), dimset.end())
    , entry(CacheEntryMap())
    {
    }

    //a cache level with no dimensions, entry must be another CacheEntry
    CacheLevel();

    std::vector<dimension_index> dims;
    CacheLevelNode entry;
  };
  
  class Cache
  {
    public:

    Cache();
    
    ~Cache();
    
    Constant
    get(const Context& k, const Delta& delta);

    void
    set(const Context& k, const Delta& delta, const Constant& value);

    void
    garbageCollect();

    void
    updateRetirementAge(int ageSeen);

    int
    retirementAge()
    {
      return m_retirementAge;
    }

    void
    miss()
    {
      ++m_misses;
    }

    void
    hit()
    {
      ++m_hits;
    }

    int
    misses() const
    {
      return m_misses;
    }

    int
    hits() const
    {
      return m_hits;
    }

    private:
    CacheLevel *m_entry;

    int m_retirementAge;

    int m_misses;
    int m_hits;
  };

  namespace Workshops
  {
    class CacheWS : public WS
    {
      public:

      CacheWS(WS* expr, u32string name, System& system)
      : m_expr(expr), m_name(std::move(name)), m_system(system)
      {}

      Constant
      operator()(Context& kappa);

      Constant
      operator()(Context& kappa, Context& delta);

      TimeConstant
      operator()(Context& kappa, Delta& d, const Thread& w, size_t t);

      void
      garbageCollect()
      {
        return m_cache.garbageCollect();
      }

      const Cache&
      getCache() const
      {
        return m_cache;
      }

      private:

      Cache m_cache;
      WS* m_expr;
      u32string m_name;

      //we need to hold onto the system to see if we should use the cache
      System& m_system;
    };
  }
}

#endif // CACHE_HPP_INCLUDED
