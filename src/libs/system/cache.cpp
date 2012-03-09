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
#include <tl/types/calc.hpp>
#include <tl/types/demand.hpp>

namespace TransLucid
{

namespace
{
  Constant
  lookup_entry_map
  (
    std::vector<dimension_index>::const_iterator iter,
    std::vector<dimension_index>::const_iterator end,
    decltype(CacheEntryMap::entry)& entry,
    Context& delta
  );

  Constant
  set_calc
  (
    std::map<Constant, CacheLevelNode>& entry,
    std::vector<dimension_index>::const_iterator iter,
    std::vector<dimension_index>::const_iterator end,
    Context& delta
  );

  struct get_cache_level_visitor
  {
    typedef Constant result_type;

    Constant
    operator()
    (
      CacheEntryMap& entry, 
      std::vector<dimension_index>::const_iterator iter,
      std::vector<dimension_index>::const_iterator end,
      Context& delta
    ) const;

    Constant
    operator()
    (
      CacheEntry& entry, 
      std::vector<dimension_index>::const_iterator iter,
      std::vector<dimension_index>::const_iterator end,
      Context& delta
    ) const;
  };

  struct get_cache_entry_visitor
  {
    typedef Constant result_type;

    Constant
    operator()(const Constant& c, Context& delta) const
    {
      return c;
    }

    Constant
    operator()(CacheLevel& l, Context& delta) const
    {
      std::vector<dimension_index> demands;
      for (auto d : l.dims)
      {
        if (!delta.has_entry(d))
        {
          demands.push_back(d);
        }
      }

      if (demands.size() > 0)
      {
        return Types::Demand::create(demands);
      }
      else
      {
        //l.entry is a CacheEntryMap
        //so look up the first dimension
        return lookup_entry_map(l.dims.begin(), l.dims.end(), l.entry.entry, 
          delta);

        //return apply_visitor(cache_level_visitor(), l.entry,
        //  demands.begin(), demands.end(), delta);
      }
    }
  };

Constant
lookup_entry_map
(
  std::vector<dimension_index>::const_iterator iter,
  std::vector<dimension_index>::const_iterator end,
  decltype(CacheEntryMap::entry)& entry,
  Context& delta
)
{
  //find the value of the current dimension in the map

  Constant c = delta.lookup(*iter);

  auto entryiter = entry.find(c);

  if (entryiter == entry.end())
  {
    return set_calc(entry, iter, end, delta);
  }
  else
  {
    return apply_visitor(get_cache_level_visitor(), entryiter->second.entry,
      iter, end, delta);
  }
}

Constant
set_calc
(
  std::map<Constant, CacheLevelNode>& entry,
  std::vector<dimension_index>::const_iterator iter,
  std::vector<dimension_index>::const_iterator end,
  Context& delta
)
{
  //iter will be the current dimension to consider
  //add an entry for delta(iter), we have already ensured that all the
  //dimensions needed exist

  Constant val = delta.lookup(*iter);

  //if iter is the last thing, the end of this needs to be calc, otherwise
  //it's another map, and repeat

  ++iter;
  if (iter == end)
  {
    Constant calc = Types::Calc::create();
    entry.insert(std::make_pair(val, CacheLevelNode(CacheEntry(calc))));
    return calc;
  }
  else
  {
    CacheLevelNode node{CacheEntryMap()};
    auto inserted = entry.insert(std::make_pair(val, node));

    set_calc
    (
      get<CacheEntryMap>(inserted.first->second.entry).entry,
      ++iter,
      end,
      delta
    );
  }
}

Constant
get_cache_level_visitor::operator()
(
  CacheEntryMap& entry, 
  std::vector<dimension_index>::const_iterator iter,
  std::vector<dimension_index>::const_iterator end,
  Context& delta
) const
{
  //we are ready to look at the next dimension

  //if iter == end then we have too many entries
  if (iter == end)
  {
    throw __FILE__ ": " STRING_(__LINE__) ": Cache error!";
  }

  //otherwise increment iter and look again
  return lookup_entry_map(++iter, end, entry.entry, delta);
}

Constant
get_cache_level_visitor::operator()
(
  CacheEntry& entry, 
  std::vector<dimension_index>::const_iterator iter,
  std::vector<dimension_index>::const_iterator end,
  Context& delta
) const
{
  if (iter != end)
  {
    //somehow we got to the end of the entries without looking at all the
    //dimensions, something screwed up
    throw __FILE__ ": " STRING_(__LINE__) ": Cache error!";
  }
  
  //otherwise we are ready to look at the next level
  return apply_visitor(get_cache_entry_visitor(), entry.entry, delta);
}

}

Constant
Cache::get(Context& delta)
{
  return apply_visitor(get_cache_entry_visitor(), m_entry.entry, delta);
}

void
Cache::set(const Context& delta, const Constant& value)
{
}

}
