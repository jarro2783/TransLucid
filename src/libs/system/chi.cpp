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
 * @file chi.cpp
 * The CHI dimension implementation.
 */

#include <tl/chi.hpp>
#include <tl/system.hpp>

namespace TransLucid
{
    
dimension_index
ChiMap::lookup(const ChiDim& d)
{
  auto iter = m_data.find(d);

  if (iter != m_data.end())
  {
    return iter->second;
  }
  else
  {
    auto dim = m_system.nextHiddenDim();
    m_data.insert(std::make_pair(d, dim));

    return dim;
  }
}

}
