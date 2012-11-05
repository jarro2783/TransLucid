/* Run time any dimensional array hyperdaton.
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

#include <vector>
#include <utility>

#include <tl/hyperdaton.hpp>

namespace TransLucid
{
  class ArrayHD : public IOHD
  {
    public:

    ArrayHD()
    : IOHD(1), m_size(0), m_data(nullptr)
    {}

    void
    initialise(const std::vector<std::pair<dimension_index, size_t>>& bounds);

    Constant*
    begin()
    {
      return m_data;
    }

    const Constant*
    begin() const
    {
      return m_data;
    }

    Constant*
    end()
    {
      return m_data + m_size;
    }

    const Constant*
    end() const
    {
      return m_data + m_size;
    }

    Constant
    get(const Context& k) const;

    void
    put(const Context&, const Constant&);

    void
    commit();

    Region
    variance() const;

    void
    addAssignment(const Tuple&)
    {
      //ignore this
    }

    private:
    size_t m_size;
    Constant* m_data;
    std::vector<std::pair<dimension_index, size_t>> m_bounds;
    std::vector<size_t> m_multipliers;
    Region m_variance;

    public:

    const decltype(m_bounds)&
    bounds() const
    {
      return m_bounds;
    }

  };
}
