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
#include <tl/types/demand.hpp>

namespace TransLucid
{

namespace
{
  struct cache_entry_visitor
  {
    typedef Constant result_type;

    Constant
    operator()(const Constant& c, Context& delta) const
    {
      return c;
    }

    Constant
    operator()(const CacheLevel& l, Context& delta) const
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
    }
  };
}

Constant
Cache::get(Context& delta) const
{
  return apply_visitor(cache_entry_visitor(), m_entry, delta);
}

}
