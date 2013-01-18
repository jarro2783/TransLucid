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
#include <tl/system.hpp>
#include <tl/types/calc.hpp>
#include <tl/types/demand.hpp>
#include <tl/types/special.hpp>
#include <tl/types_util.hpp>

#include <tl/output.hpp>

#include <iostream>

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
    Context& delta,
    Cache& cache
  );

  Constant
  set_calc
  (
    std::map<Constant, CacheLevelNode>& entry,
    std::vector<dimension_index>::const_iterator iter,
    std::vector<dimension_index>::const_iterator end,
    Context& delta
  );

  bool
  collect_entry_map(CacheEntryMap& entrymap, Cache& cache);

  struct get_cache_level_visitor
  {
    typedef Constant result_type;

    Constant
    operator()
    (
      CacheEntryMap& entry, 
      std::vector<dimension_index>::const_iterator iter,
      std::vector<dimension_index>::const_iterator end,
      Context& delta,
      Cache& cache
    ) const;

    Constant
    operator()
    (
      CacheEntry& entry, 
      std::vector<dimension_index>::const_iterator iter,
      std::vector<dimension_index>::const_iterator end,
      Context& delta,
      Cache& cache
    ) const;
  };

  struct get_cache_entry_visitor
  {
    typedef Constant result_type;

    Constant
    operator()(const Constant& c, Context& delta, Cache& cache) const
    {
      cache.hit();

      if (c.index() == TYPE_INDEX_CALC)
      {
        //std::cerr << "calc already in cache" << std::endl;
        return Types::Special::create(SP_LOOP);
      }
      return c;
    }

    Constant
    operator()(CacheLevel& l, Context& delta, Cache& cache) const
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
        //return lookup_entry_map(l.dims.begin(), l.dims.end(), l.entry.entry, 
        //  delta, cache);

        return apply_visitor(get_cache_level_visitor(), l.entry.entry,
          l.dims.begin(), l.dims.end(), delta, cache);
      }
    }
  };

  struct set_level_node
  {
    typedef void result_type;

    void
    operator()
    (
      CacheEntry& entry, 
      const Context& delta, 
      const Constant& value,
      std::vector<dimension_index>::const_iterator begin,
      std::vector<dimension_index>::const_iterator end
    ) const;

    void
    operator()
    (
      CacheEntryMap& entrymap, 
      const Context& delta, 
      const Constant& value,
      std::vector<dimension_index>::const_iterator begin,
      std::vector<dimension_index>::const_iterator end
    ) const;
  };

  struct collect_level_node
  {
    typedef bool result_type;

    bool
    operator()(CacheEntry& entry, Cache& cache) const;

    bool
    operator()(CacheEntryMap& entrymap, Cache& cache) const
    {
      return collect_entry_map(entrymap, cache);
    }
  };

  struct collect_entry
  {
    typedef bool result_type;

    bool
    operator()(const Constant& c, Cache& cache) const
    {
      //a constant can always be collected
      return true;
    }

    bool
    operator()(CacheLevel& level, Cache& cache) const
    {
      return apply_visitor(collect_level_node(), level.entry.entry, cache);
    }
  };

bool
collect_entry_map(CacheEntryMap& entrymap, Cache& cache)
{
  //visit every child in the map
  auto iter = entrymap.entry.begin();

  while (iter != entrymap.entry.end())
  {
    bool result = apply_visitor(collect_level_node(), iter->second.entry, 
      cache);

    if (result)
    {
      //the entry below can be collected so we can delete the entry
      auto next = iter;
      ++next;
      entrymap.entry.erase(iter);
      iter = next;
    }
    else
    {
      ++iter;
    }
  }

  //this node can be collected if it is empty
  return entrymap.entry.empty();
}

bool
collect_level_node::operator()(CacheEntry& entry, Cache& cache) const
{
  //we can collect this entry if its children can be collected and
  //its age is greater than the retirement age

  bool result = apply_visitor(collect_entry(), entry.entry, cache);
  bool collect = false;

  if (result && entry.age > cache.retirementAge())
  {
    collect = true;
  }
  else
  {
    //it survived
    ++entry.age;
  }

  return collect;
}

Constant
lookup_entry_map
(
  std::vector<dimension_index>::const_iterator iter,
  std::vector<dimension_index>::const_iterator end,
  decltype(CacheEntryMap::entry)& entry,
  Context& delta,
  Cache& cache
)
{
  //find the value of the current dimension in the map

  Constant c = delta.lookup(*iter);

  auto entryiter = entry.find(c);

  if (entryiter == entry.end())
  {
    cache.miss();
    return set_calc(entry, iter, end, delta);
  }
  else
  {
    return apply_visitor(get_cache_level_visitor(), entryiter->second.entry,
      ++iter, end, delta, cache);
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
      iter,
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
  Context& delta,
  Cache& cache
) const
{
  //we are ready to look at the next dimension

  //if iter == end then we have too many entries
  if (iter == end)
  {
    throw __FILE__ ": " STRING_(__LINE__) ": Cache error!";
  }

  //otherwise look again at the next entry
  return lookup_entry_map(iter, end, entry.entry, delta, cache);
}

Constant
get_cache_level_visitor::operator()
(
  CacheEntry& entry, 
  std::vector<dimension_index>::const_iterator iter,
  std::vector<dimension_index>::const_iterator end,
  Context& delta,
  Cache& cache
) const
{
  if (iter != end)
  {
    //somehow we got to the end of the entries without looking at all the
    //dimensions, something screwed up
    throw __FILE__ ": " STRING_(__LINE__) ": Cache error!";
  }

  //update the age of this node
  cache.updateRetirementAge(entry.age);
  entry.age = 0;
  
  //otherwise we are ready to look at the next level
  return apply_visitor(get_cache_entry_visitor(), entry.entry, delta, cache);
}

void
set_visit_top_entry
(
  CacheEntry& entry, 
  const Context& delta,
  const Constant& value
);

//overwrites entry with value
//but if value is a demand it becomes a new CacheLevel
void
set_cache_value
(
  CacheEntry& entry,
  const Constant& value
)
{
  if (value.index() == TYPE_INDEX_DEMAND)
  {
    const DemandType& demand = Types::Demand::get(value);
    const auto& demandSet = demand.dims();

    entry.entry = CacheLevel{demandSet};
  }
  else
  {
    entry.entry = value;
  }
}

void
set_level_node::operator()
(
  CacheEntry& entry, 
  const Context& delta, 
  const Constant& value,
  std::vector<dimension_index>::const_iterator begin,
  std::vector<dimension_index>::const_iterator end
) const
{
  //check consistency
  if (begin != end)
  {
    throw __FILE__ ": " STRING_(__LINE__) 
          ": Cache error, entry reached before dims ran out";
  }

  set_visit_top_entry(entry, delta, value);
}


void
set_level_node::operator()
(
  CacheEntryMap& entrymap, 
  const Context& delta, 
  const Constant& value,
  std::vector<dimension_index>::const_iterator begin,
  std::vector<dimension_index>::const_iterator end
) const
{
  if (begin == end)
  {
    throw __FILE__ ": " STRING_(__LINE__) 
          ": Cache error, traversing map but no more dims";
  }

  auto iter = entrymap.entry.find(delta.lookup(*begin));

  if (iter == entrymap.entry.end())
  {
    throw __FILE__ ": " STRING_(__LINE__) 
          ": Cache error, there isn't already a calc for this entry";
  }

  apply_visitor(*this, iter->second.entry, delta, value, ++begin, end);
}

void
set_visit_top_entry
(
  CacheEntry& entry, 
  const Context& delta,
  const Constant& value
)
{
  //There is guaranteed to be an entry for the current delta which is set to
  //calc. If value is a demand, that becomes the next level in the cache
  //hierarchy.

  //apply_visitor(set_cache_entry_visitor(), m_entry.entry, delta, value);

  const Constant* c = TransLucid::get<Constant>(&entry.entry);
  CacheLevel* level = TransLucid::get<CacheLevel>(&entry.entry);

  if (c != nullptr)
  {
    //overwrite it
    set_cache_value(entry, value);
  }
  else if (level != nullptr)
  {
    //traverse the level
    apply_visitor(set_level_node(), level->entry.entry, delta, value,
      level->dims.begin(), level->dims.end());
  }
}

}

Cache::Cache()
: m_entry(nullptr)
, m_retirementAge(2)
{
}

Cache::~Cache()
{
  delete m_entry;
}

Constant
Cache::get(Context& delta)
{
  if (!m_entry)
  {
    m_entry = new CacheLevel;
    return Types::Calc::create();
  }
  else
  {
    return get_cache_entry_visitor()(*m_entry, delta, *this);
  }
}

void
Cache::set(const Context& delta, const Constant& value)
{
  if (!m_entry)
  {
    throw __FILE__ ": " STRING_(__LINE__) ": Can't set empty cache";
  }

  //set_visit_top_entry(m_entry, delta, value);
  apply_visitor
  (
    set_level_node(), 
    m_entry->entry.entry, 
    delta, 
    value, 
    m_entry->dims.begin(),
    m_entry->dims.end()
  );
}

void
Cache::garbageCollect()
{
  if (m_entry != nullptr)
  {
    apply_visitor(collect_level_node(), m_entry->entry.entry, *this);
    //finally, decrease the retirement age
    --m_retirementAge;
  }
}

void
Cache::updateRetirementAge(int ageSeen)
{
  if (ageSeen > m_retirementAge)
  {
    m_retirementAge = ageSeen;
  }
}

CacheLevel::CacheLevel()
: entry(CacheEntry(Types::Calc::create()))
{
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
  if (m_system.cacheEnabled())
  {
    //we need to start a new thread and a new time
    Thread w;
    Delta delta;
    auto c = operator()(kappa, delta, w, 0);

    if (c.second.index() == TYPE_INDEX_DEMAND)
    {
      ContextPerturber p{kappa};
      //or not, we need to fill in the undefined dimensions and try again
      while (c.second.index() == TYPE_INDEX_DEMAND)
      {
        //std::cerr << "a demand got through" << std::endl;

        for (auto d : get_constant_pointer<DemandType>(c.second).dims())
        {
          p.perturb(d, kappa.lookup(d));
          delta.insert(d);
          //std::cerr << d << std::endl;
          //std::cerr << "in context: " << kappa.has_entry(d) << std::endl;
        }

        c = operator()(kappa, delta, w, 0);
      }
    }

    return c.second;
  }
  else
  {
    //we're not using the cache at the moment
    return (*m_expr)(kappa);
  }
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
      //std::cerr << "cache node: " << m_name << ": calc" << std::endl;
      d = (*m_expr)(kappa, subdelta);
      m_cache.set(subdelta, d);
    }

    if (d.index() == TYPE_INDEX_DEMAND)
    {
      //std::cerr << "cache node: " << m_name << ": demands:";
      const auto& demands = Types::Demand::get(d);

      for (auto dim : demands.dims())
      {
        //std::cerr << " " << dim;
        p.perturb(dim, kappa.lookup(dim));
      }
      //std::cerr << std::endl;
    }
    else
    {
      break;
    }
  }

  Constant v = m_cache.get(delta);

  //if (v.index() == TYPE_INDEX_SPECIAL && get_constant<Special>(v) == SP_LOOP)
  //{
    //std::cerr << m_name << " loop" << std::endl;
  //}

  return v;
}

TimeConstant
CacheWS::operator()(Context& kappa, Delta& delta, const Thread& w, size_t t)
{
  Delta subdelta;
  Context subcontext;
  ContextPerturber p(subcontext);

  while (true)
  {
    Constant d = m_cache.get(subcontext);

    if (d.index() == TYPE_INDEX_CALC)
    {
      //std::cerr << "cache node: " << m_name << ": calc" << std::endl;
      auto result = (*m_expr)(kappa, subdelta, w, t);
      m_cache.set(subcontext, result.second);
    }

    if (d.index() == TYPE_INDEX_DEMAND)
    {
      //std::cerr << "cache node: " << m_name << ": demands:";
      const auto& demands = Types::Demand::get(d);

      subdelta.insert(demands.dims().begin(), demands.dims().end());

      for (auto dim : demands.dims())
      {
        //std::cerr << " " << dim;
        p.perturb(dim, kappa.lookup(dim));
      }
      //std::cerr << std::endl;
    }
    else
    {
      break;
    }
  }

  Context finalk;
  ContextPerturber p2(finalk);

  for (auto dim : delta)
  {
    p2.perturb(dim, kappa.lookup(dim));
  }

  Constant v = m_cache.get(finalk);

  //if (v.index() == TYPE_INDEX_SPECIAL && get_constant<Special>(v) == SP_LOOP)
  //{
    //std::cerr << m_name << " loop" << std::endl;
  //}

  return std::make_pair(t, v);
}

}

}
