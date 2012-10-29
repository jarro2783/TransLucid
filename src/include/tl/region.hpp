/* A region.
   Copyright (C) 2009-2012 Jarryd Beck

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

/**
 * @file region.hpp
 * The region object.
 */

#ifndef TL_REGION_HPP_INCLUDED
#define TL_REGION_HPP_INCLUDED

#include <utility>
#include <vector>

#include <tl/types.hpp>

namespace TransLucid
{
  class Region
  {
    public:

    enum class Containment
    {
      IS,
      IN,
      IMP
    };

    typedef std::tuple<Constant, Containment, Constant> Entry;
    typedef std::vector<Entry> Entries;

    Region(Entries entries)
    : m_entries(entries)
    {
    }

    bool
    operator==(const Region& rhs) const
    {
      if (m_entries.size() != rhs.m_entries.size())
      {
        return false;
      }

      auto liter = m_entries.begin();
      auto riter = rhs.m_entries.begin();

      while (liter != m_entries.end())
      {
        const auto& lval = *liter;
        const auto& rval = *riter;

        if (std::get<0>(lval) != std::get<0>(rval))
        {
          return false;
        }

        if (std::get<1>(lval) != std::get<1>(rval))
        {
          return false;
        }

        if (std::get<2>(lval) != std::get<2>(rval))
        {
          return false;
        }

        ++liter;
        ++riter;
      }

      return true;
    }

    bool
    operator<(const Region& rhs) const
    {
    }

    private:
    Entries m_entries;
  };
}

#endif
