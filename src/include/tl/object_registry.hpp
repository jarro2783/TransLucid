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
#include <initializer_list>

namespace TransLucid
{
  template <typename T>
  struct Increment
  {
    void
    operator()(T& i)
    {
      ++i;
    }
  };

  template <typename T>
  struct Decrement
  {
    void
    operator()(T& i)
    {
      --i;
    }
  };

  template <typename T, typename Index, typename Next = Increment<Index>>
  class ObjectRegistry
  {
    public:
    ObjectRegistry(Index& index)
    : m_index(index)
    {
    }

    template <typename List>
    ObjectRegistry(Index& index, const List& initial)
    : m_index(index)
    {
      for (auto i : initial)
      {
        m_objects.insert(i);
        m_reverse.insert({i.second, i.first});
      }
    }

    Index
    operator()(const T& v)
    {
      auto result = m_objects.insert(std::make_pair(v, m_index));
      if (result.second)
      {
        m_reverse.insert({m_index, v});
        m_next(m_index);
      }
      return result.first->second;
    }

    private:

    typedef std::unordered_map<T, Index> ObjectMap;
    typedef std::unordered_map<Index, T> ReverseMap;

    ObjectMap m_objects;
    ReverseMap m_reverse;

    Index& m_index;
    Next m_next;

    public:

    const T* 
    reverseLookup(const Index& index) const
    {
      auto iter = m_reverse.find(index);
      if (iter == m_reverse.end())
      {
        return nullptr;
      }
      else
      {
        return &iter->second;
      }
    }

  };
}

#endif
