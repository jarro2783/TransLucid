/* The context.
   Copyright (C) 2009-2011 Jarryd Beck and John Plaice

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
 * @file context.cpp
 * The context object.
 */

#include <tl/context.hpp>
#include <tl/types/special.hpp>

namespace TransLucid
{

Context::Context()
: m_min(0), m_max(0),
 m_all(Types::Special::create(SP_DIMENSION))
{
}

const Constant&
Context::lookup(dimension_index dim)
{
  if (dim <= m_min || dim >= m_max)
  {
    return m_all;
  }
  else
  {
    const auto& s = m_context[makeIndex(dim)];

    if (s.empty())
    {
      return m_all;
    }
    else
    {
      return s.top();
    }
  }

  return m_all;
}

void
Context::restore(const Tuple& t)
{
  for (const auto& v : t)
  {
    m_context[makeIndex(v.first)].pop();
  }
}

void
Context::perturb(const Tuple& t)
{
  for (const auto& v : t)
  {
    if (v.first == 0)
    {
    }
  }
}

}
