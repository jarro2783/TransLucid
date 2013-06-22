/* Type context.
   Copyright (C) 2013 Jarryd Beck

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

#include <tl/tyinf/type_context.hpp>

namespace TransLucid
{

namespace TypeInference
{

void
TypeContext::add(dimension_index d, Type t)
{
  m_lambdas[d] = t;
}

void
TypeContext::join(const TypeContext& other)
{
  for (const auto& p : other.m_lambdas)
  {
    auto iter = m_lambdas.find(p.first);

    if (iter != m_lambdas.end())
    {
      iter->second = construct_glb(p.second, iter->second);
    }
    else
    {
      m_lambdas.insert(p);
    }
  }

  for (const auto& p : other.m_vars)
  {
    auto iter = m_vars.find(p.first);

    if (iter != m_vars.end())
    {
      iter->second = construct_glb(p.second, iter->second);
    }
    else
    {
      m_vars.insert(p);
    }
  }
}

Type
TypeContext::lookup(dimension_index d)
{
  auto iter = m_lambdas.find(d);
  if (iter != m_lambdas.end())
  {
    return iter->second;
  }
  else
  {
    return TypeTop();
  }
}

}
}
