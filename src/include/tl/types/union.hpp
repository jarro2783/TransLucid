/* The union type.
   Copyright (C) 2011, 2012 Jarryd Beck

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

#ifndef TL_TYPES_UNION_HPP_INCLUDED
#define TL_TYPES_UNION_HPP_INCLUDED

#include <tl/types.hpp>

#include <set>

namespace TransLucid
{
  class UnionType
  {
    public:

    void
    append(const UnionType& u);

    void
    append(const Constant& c);

    bool
    operator==(const UnionType& rhs) const
    {
      return m_types == rhs.m_types;
    }

    bool
    operator<(const UnionType& rhs) const
    {
      return m_types < rhs.m_types;
    }

    size_t
    hash() const;

    private:
    std::set<Constant> m_types;
  };

  namespace Types
  {
    namespace Union
    {
      Constant
      create(const Constant& lhs, const Constant& rhs);

      const UnionType&
      get(const Constant& c);

      bool 
      equality(const Constant& lhs, const Constant& rhs);

      size_t
      hash(const Constant& c);

      bool
      less(const Constant& lhs, const Constant& rhs);
    }
  }
}

#endif
