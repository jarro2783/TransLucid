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
 * @file region.cpp
 * The region object.
 */

#include <tl/region.hpp>

namespace TransLucid
{

bool
Region::operator==(const Region& rhs) const
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
Region::operator<(const Region& rhs) const
{
  auto iterl = m_entries.begin();
  auto iterr = rhs.m_entries.begin();

  while (iterl != m_entries.end() && iterr != rhs.m_entries.end())
  {
    if (std::get<0>(*iterl) < std::get<0>(*iterr))
    {
      return true;
    }
    else if (std::get<0>(*iterr) < std::get<0>(*iterl))
    {
      return false;
    }
    else if (std::get<1>(*iterl) < std::get<1>(*iterr))
    {
      return true;
    }
    else if (std::get<1>(*iterr) < std::get<1>(*iterl))
    {
      return false;
    }
    else if (std::get<2>(*iterl) < std::get<2>(*iterr))
    {
      return true;
    }
    else if (std::get<2>(*iterr) < std::get<2>(*iterl))
    {
      return false;
    }
    //this pair is equal, go to the next one
    ++iterl;
    ++iterr;
  }

  if (iterl == m_entries.end() && iterr == rhs.m_entries.end())
  {
    //they must be equal, so return false
    return false;
  }
  else if (iterl == m_entries.end())
  {
    return true;
  }
  else
  {
    return false;
  }
}

size_t
Region::hash() const
{
  size_t value = 0;

  for (const auto& entry : m_entries)
  {
    std::_Hash_impl::__hash_combine(std::get<0>(entry), value);
    std::_Hash_impl::__hash_combine(std::get<1>(entry), value);
    std::_Hash_impl::__hash_combine(std::get<2>(entry), value);
  }

  return value;
}

}
