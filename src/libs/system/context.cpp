/* The context.
   Copyright (C) 2009-2013 Jarryd Beck

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

#include <tl/fixed_indexes.hpp>

#include <tl/context.hpp>
#include <tl/types/special.hpp>

#include <algorithm>
#include <iostream>

namespace TransLucid
{

namespace
{
  static const int DEFAULT_MIN = -1;
  static const int DEFAULT_MAX = 0;
}

Context::Context()
: m_min(DEFAULT_MIN), m_max(DEFAULT_MAX),
 m_all(Types::Special::create(SP_DIMENSION))
{
}

//TODO a more specialised implementation of this
Context::Context(const Tuple& k)
: m_min(DEFAULT_MIN), m_max(DEFAULT_MAX),
 m_all(Types::Special::create(SP_DIMENSION))
{
  for (auto v : k)
  {
    perturb(v.first, v.second);
  }
}

void
Context::restore(const Tuple& t)
{
  for (const auto& v : t)
  {
    auto index = makeIndex(v.first);
    m_context[index].second.pop();

    if (!m_context[index].second.empty())
    {
      m_context[index].first = m_context[index].second.top();
    }
    else
    {
      m_context[index].first = m_all;
    }
  }
}

void
Context::perturb(const Tuple& t)
{
  for (const auto& v : t)
  {
    perturb(v.first, v.second);
  }
}

void
Context::perturb(dimension_index d, const Constant& c)
{
  if (c.index() == TYPE_INDEX_DEMAND)
  {
    std::cerr << "internal compiler error at " 
              << __FILE__ << ": " << STRING_(__LINE__) << std::endl
              << "context perturbed by a demand" << std::endl;
  }

  //do we need to allocate some more slots
  //putting max first means that if the 0th is added first it will be pushed
  //back, this might be slightly better than pushing front first
  if (d >= m_max)
  {
    std::fill_n(std::back_inserter(m_context), d - m_max + 1,
      std::make_pair(Types::Special::create(SP_DIMENSION), 
        std::stack<Constant>()));
    m_max = d + 1;
  }
  else if (d <= m_min)
  {
    std::fill_n(std::front_inserter(m_context), m_min - d + 1,
      std::make_pair(Types::Special::create(SP_DIMENSION), 
        std::stack<Constant>()));
    m_min = d - 1;
  }

  auto index = makeIndex(d);
  m_context[index].second.push(c);
  m_context[index].first = c;

  m_setDims.insert(d);
}

void
Context::reset()
{
  m_context.clear();
  m_min = DEFAULT_MIN;
  m_max = DEFAULT_MAX;
}

Context::operator Tuple() const
{
  tuple_t t;

  for (dimension_index d = m_min + 1; d != m_max; ++d)
  {
    const auto& s = m_context[makeIndex(d)];
    if (!s.second.empty())
    {
      t.insert(std::make_pair(d, s.second.top()));
    }
  }

  return Tuple(t);
}

bool
Context::operator<=(const Context& rhs) const
{
  //rhs should have more or the same things defined as this, and for those that
  //are defined, they should be equal

  //check if anything is defined outside of rhs's min
  dimension_index current = m_min + 1; 
  dimension_index index = makeIndex(current);
  if (m_min < rhs.m_min)
  {
    while(current != rhs.m_min + 1)
    {
      if (!m_context[index].second.empty())
      {
        return false;
      }

      ++current;
      ++index;
    }
  }

  //now we are at the first thing in rhs
  dimension_index last = m_max < rhs.m_max ? m_max : rhs.m_max;
  dimension_index rhsIndex = rhs.makeIndex(current);

  while (current != last)
  {
    const auto& ls = m_context[index];
    const auto& rs = rhs.m_context[rhsIndex];
    if (!ls.second.empty())
    {
      if (!rs.second.empty())
      {
        const Constant& lc = ls.second.top();
        const Constant& rc = rs.second.top();

        if (lc != rc)
        {
          return false;
        }
      }
      else
      {
        return false;
      }
    }

    ++index;
    ++current;
    ++rhsIndex;
  }

  //check that this has nothing else
  if (m_max > rhs.m_max)
  {
    while (current < m_max)
    {
      if (!m_context[index].second.empty())
      {
        return false;
      }
      ++index;
      ++current;
    }
  }

  return true;
}

}
