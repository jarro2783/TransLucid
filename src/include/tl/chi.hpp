/* CHI dimension implementation
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
 * @file chi.hpp
 * The CHI dimension implementation.
 */

#ifndef TL_CHI_HPP_INCLUDED
#define TL_CHI_HPP_INCLUDED

#include <tl/types.hpp>
#include <tl/utility.hpp>

#include <cstdint>
#include <cstring>
#include <functional>
#include <iostream>
#include <unordered_map>
#include <vector>

namespace TransLucid
{
  class ChiDim
  {
    public:
    typedef int16_t type_t;

    ChiDim(int which, const std::vector<type_t>& stack)
    : m_which(which)
    , m_stack(stack)
    {
    }

    size_t
    hash() const
    {
      size_t value = m_which;

      for (auto v : m_stack)
      {
        hash_combine(v, value);
      }

      return value;
    }

    bool
    operator==(const ChiDim& rhs) const
    {
      return m_which == rhs.m_which && m_stack == rhs.m_stack;
    }

    private:

    int m_which;
    std::vector<type_t> m_stack;

    friend
    std::ostream&
    operator<<(std::ostream& os, const ChiDim& chi);
  };

  inline
  std::ostream&
  operator<<(std::ostream& os, const ChiDim& chi)
  {
    os << chi.m_which << ", ";
    for (auto v : chi.m_stack)
    {
      os << static_cast<int>(v) << ":";
    }

    return os;
  }
}

namespace std
{
  template <>
  struct hash<TransLucid::ChiDim>
  {
    size_t
    operator()(const TransLucid::ChiDim& d) const
    {
      return d.hash();
    }
  };
}

namespace TransLucid
{
  class System;

  class ChiMap
  {
    public:
    ChiMap(System& s)
    : m_system(s)
    {
    }
    
    dimension_index
    lookup(const ChiDim& d);

    private:
    std::unordered_map<ChiDim, dimension_index> m_data;

    System& m_system;
  };
}

#endif
