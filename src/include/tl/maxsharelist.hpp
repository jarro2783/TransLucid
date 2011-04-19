/* A maximal sharing list.
   Copyright (C) 2011 Jarryd Beck

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

#include <memory>

namespace TransLucid
{
  template <typename T>
  class MaxShareList
  {
    public:
    typedef typename std::shared_ptr<MaxShareList<T>> Ptr;

    MaxShareList()
    : m_head(nullptr)
    {
    }

    MaxShareList(T* t)
    : m_head(t)
    {
    }

    bool empty()
    {
      return m_head == nullptr;
    }

    T* head() const
    {
      return m_head;
    }

    Ptr tail() const
    {
      return m_tail;
    }

    Ptr cons(T* t)
    {
      return MaxShareList<T>(t, Ptr(*this));
    }

    private:

    MaxShareList(T* t, const Ptr& tail)
    : m_head(t)
    , m_tail(tail)
    {
    }

    T* m_head;
    Ptr m_tail;
  };
}
