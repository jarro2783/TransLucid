/* The demand type.
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

#ifndef TL_TYPES_DEMAND_HPP_INCLUDED
#define TL_TYPES_DEMAND_HPP_INCLUDED

#include <vector>

#include <tl/types.hpp>
#include <functional>

namespace TransLucid
{
  class DemandType
  {
    public:
    DemandType(const std::vector<dimension_index>& dims)
    : m_dims(dims)
    {
    }

    const std::vector<dimension_index>&
    dims() const
    {
      return m_dims;
    }

    bool
    operator==(const DemandType& rhs) const
    {
      return m_dims == rhs.m_dims;
    }

    size_t
    hash() const
    {
      size_t h = 0;

      for (const auto& v : m_dims)
      {
        std::_Hash_impl::__hash_combine(v, h);
      }

      return h;
    }

    private:
    std::vector<dimension_index> m_dims;
  };

  namespace Types
  {
    namespace Demand
    {
      Constant
      create(const std::vector<dimension_index>& dims);

      bool
      equality(const Constant& lhs, const Constant& rhs);

      size_t
      hash(const Constant& c);

      const DemandType&
      get(const Constant& c);
    }
  }
}

#endif
