/* The warehouse.
   Copyright (C) 2012 Jarryd Beck

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

    return set_calc
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
  //There is guaranteed to be an entry for the current delta which is set to
  //calc. If value is a demand, that becomes the next level in the cache
  //hierarchy.
}

namespace Workshops
{

Constant
CacheWS::operator()(Context& kappa)
{
  //the link between cached and uncached code, the difference between this
  //and the cached evaluator is that this one fills in all the dimensions
  //requested from kappa and doesn't return them

  //although maybe it can just call cached code with all the dimensions
  return operator()(kappa, kappa);
}

Constant
CacheWS::operator()(Context& kappa, Context& delta)
{
  Context subdelta;
  ContextPerturber p(subdelta);

  while (true)
  {
    Constant d = m_cache.get(subdelta);

    if (d.index() == TYPE_INDEX_CALC)
    {
      d = (*m_expr)(kappa, subdelta);
      m_cache.set(subdelta, d);
    }

    if (d.index() == TYPE_INDEX_DEMAND)
    {
      const auto& demands = Types::Demand::get(d);

      for (auto dim : demands.dims())
      {
        p.perturb(dim, kappa.lookup(dim));
      }
    }
    else
    {
      break;
    }
  }

  return m_cache.get(delta);
}

}

}
