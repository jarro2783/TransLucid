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
#include <tl/types/union.hpp>
#include <tl/types_util.hpp>
#include <tl/types.hpp>

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
}

void
UnionType::append(const Constant& c)
{
  if (c.index() == TYPE_INDEX_UNION)
  {
    append(get_constant_pointer<UnionType>(c));
  }
  else
  {
    m_types.insert(c);
  }
}

size_t
UnionType::hash() const
{
  size_t h = 0;
  std::hash<Constant> hasher;

  for (const auto& v : m_types)
  {
    std::_Hash_impl::__hash_combine(v, hasher(v));
  }
     
  return h;
}

}
