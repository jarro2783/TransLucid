/* Generic object index generation registry.
   Copyright (C) 2011 Jarryd Beck and John Plaice

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

#ifndef TL_OBJECT_REGISTRY_HPP_INCLUDED
#define TL_OBJECT_REGISTRY_HPP_INCLUDED

#include <unordered_map>

namespace TransLucid
{
  template <typename T, typename Index>
  class ObjectRegistry
  {
    public:
    ObjectRegistry(Index& index)
    : m_index(index)
    {
    }

    Index
    operator()(const T& v)
    {
      auto result = m_objects.insert(std::make_pair(v, m_index));
      if (result.second)
      {
        ++m_index;
      }
      return result.first->second;
    }

    private:

    typedef std::unordered_map<T, Index> ObjectMap;
    ObjectMap m_objects;

    Index& m_index;
  };
}

#endif