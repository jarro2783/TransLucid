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
#include <tl/utility.hpp>

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

    if (lval.first != rval.first)
    {
      return false;
    }

    if (lval.second.first != rval.second.first)
    {
      return false;
    }

    if (lval.second.second != rval.second.second)
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
    if (iterl->first < iterr->first)
    {
      return true;
    }
    else if (iterr->first < iterl->first)
    {
      return false;
    }
    else if (iterl->second.first < iterr->second.first)
    {
      return true;
    }
    else if (iterr->second.first < iterl->second.first)
    {
      return false;
    }
    else if (iterl->second.second < iterr->second.second)
    {
      return true;
    }
    else if (iterr->second.second < iterl->second.second)
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
    hash_combine(entry.first, value);
    hash_combine(entry.second.first, value);
    hash_combine(entry.second.second, value);
  }

  return value;
}

}
