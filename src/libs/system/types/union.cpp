/* Union type.
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

#include <tl/fixed_indexes.hpp>
#include <tl/types/range.hpp>
#include <tl/types/union.hpp>
#include <tl/types_util.hpp>
#include <tl/types.hpp>
#include <tl/utility.hpp>

namespace TransLucid
{

namespace
{
  TypeFunctions union_type_functions = 
    {
      &Types::Union::equality,
      &Types::Union::hash,
      &delete_ptr<UnionType>,
      &Types::Union::less
    };
}

namespace detail
{
  template <>
  struct clone<UnionType>
  {
    UnionType*
    operator()(const UnionType& u)
    {
      return new UnionType(u);
    }
  };
}

namespace Types
{

namespace Union
{

Constant
create(const Constant& lhs, const Constant& rhs)
{
  UnionType u;
  u.append(lhs);
  u.append(rhs);

  return make_constant_pointer
    (u, &union_type_functions, TYPE_INDEX_UNION);
}

const UnionType&
get(const Constant& c)
{
  return get_constant_pointer<UnionType>(c);
}

bool 
equality(const Constant& lhs, const Constant& rhs)
{
  return get(lhs) == get(rhs);
}

size_t
hash(const Constant& c)
{
  return get(c).hash();
}

bool
less(const Constant& lhs, const Constant& rhs)
{
  return get(lhs) < get(rhs);
}

}

}

void
UnionType::append(const UnionType& u)
{
  m_types.insert(u.m_types.begin(), u.m_types.end());
  m_hasIntmp = m_hasIntmp || u.m_hasIntmp;
}

void
UnionType::append(const Constant& c)
{
  if (c.index() == TYPE_INDEX_UNION)
  {
    append(get_constant_pointer<UnionType>(c));
  }
  else if (c.index() == TYPE_INDEX_RANGE)
  {
    //search everything and see if we can join up an overlap
    //build a new range out of all the overlaps
    //then insert it at the end
    Range r = Types::Range::get(c);
    auto iter = m_types.begin();
    while (iter != m_types.end())
    {
      if (iter->index() == TYPE_INDEX_RANGE)
      {
        const Range& inSet = Types::Range::get(*iter);

        if (inSet.overlaps(r))
        {
          r = inSet.join(r);
          iter = m_types.erase(iter);
        }
        else
        {
          ++iter;
        }
      }
      else
      {
        ++iter;
      }
    }

    m_types.insert(Types::Range::create(r));
  }
  else
  {
    if (c.index() == TYPE_INDEX_TYPE && 
        get_constant<type_index>(c) == TYPE_INDEX_INTMP)
    {
      m_hasIntmp = true;
    }
    m_types.insert(c);
  }
}

size_t
UnionType::hash() const
{
  size_t h = 0;

  for (const auto& v : m_types)
  {
    hash_combine(v, h);
  }
     
  return h;
}

bool
UnionType::contains(const Constant& a) const
{
  if (a.index() == TYPE_INDEX_RANGE)
  {
    if (m_hasIntmp)
    {
      return true;
    }
  }

  for (const auto& v : m_types)
  {
    if (valueRefines(a, v))
    {
      return true;
    }
  }

  return false;
}

}
