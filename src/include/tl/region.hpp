/* A region.
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

    typedef std::tuple<dimension_index, Containment, Constant> Entry;
    typedef std::vector<Entry> Entries;

    Region(Entries entries)
    : m_entries(entries)
    {
    }

    bool
    operator==(const Region& rhs) const;

    bool
    operator<(const Region& rhs) const;

    size_t
    hash() const;

    private:
    Entries m_entries;

    public:

    Entries::const_iterator
    begin() const
    {
      return m_entries.begin();
    }

    Entries::const_iterator
    end() const
    {
      return m_entries.end();
    }

    Entries::iterator
    begin()
    {
      return m_entries.begin();
    }

    Entries::iterator
    end()
    {
      return m_entries.end();
    }

  };
}

#endif
