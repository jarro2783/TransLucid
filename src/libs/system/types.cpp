/* Basic types.
   Copyright (C) 2009, 2010 Jarryd Beck and John Plaice

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
 * @file types.cpp
 * The implementation file for the basic types.
 */

#include <tl/types.hpp>
#include <tl/range.hpp>
#include <tl/system.hpp>
#include <tl/exception.hpp>

namespace TransLucid
{

namespace 
{

template <typename T>
bool
equality(const Constant& lhs, const Constant& rhs)
{
  return get_constant<T>(lhs) == get_constant<T>(rhs);
}

template <typename T>
size_t
hash_func(const Constant& c)
{
  return std::hash<T>()(get_constant<T>(c));
}

bool (*equality_functions[TYPE_FIELD_PTR])(const Constant&, const Constant&) = 
{ 
  &equality<Special>,
  &equality<bool>,
  &equality<char32_t>,
  &equality<int8_t>,
  &equality<uint8_t>,
  &equality<int16_t>,
  &equality<uint16_t>,
  &equality<int32_t>,
  &equality<uint32_t>,
  &equality<int64_t>,
  &equality<uint64_t>,
  &equality<float>,
  &equality<double>
};

size_t (*hash_functions[TYPE_FIELD_PTR])(const Constant&) =
{
  &hash_func<Special>,
  &hash_func<bool>,
  &hash_func<int8_t>,
  &hash_func<uint8_t>,
  &hash_func<int16_t>,
  &hash_func<uint16_t>,
  &hash_func<int32_t>,
  &hash_func<uint32_t>,
  &hash_func<int64_t>,
  &hash_func<uint64_t>,
  &hash_func<float>,
  &hash_func<double>
};

}

namespace detail
{

bool
constant_equality(const Constant& lhs, const Constant& rhs)
{
  return (*equality_functions[lhs.data.field])(lhs, rhs);
}

}

Tuple::Tuple()
: m_value(new tuple_t)
{
}

Tuple::Tuple(const tuple_t& tuple)
: m_value(new tuple_t(tuple))
{
}

Tuple
Tuple::at(const tuple_t& k) const
{
  tuple_t result = *m_value;

  for (auto v : k)
  {
    result.insert(v);
  }

  return Tuple(result);
}

Tuple
Tuple::insert(size_t key, const Constant& value) const
{
  tuple_t t = *m_value;
  t.insert(std::make_pair(key, value));
  return Tuple(t);
}

void
Tuple::print(std::ostream& os) const
{
  os << "[";
  for(auto& v : *m_value)
  {
    os << v.first << ":";
    //v.second.print(os);
    os << ", ";
  }
  os << "]";
}

size_t
Constant::hash() const
{
  if (data.field == TYPE_FIELD_PTR)
  {
    return (*data.ptr->functions->hash)(*this);
  }
  else
  {
    return (*hash_functions[data.field])(*this);
  }
}

size_t
Tuple::hash() const
{
  std::hash<tuple_t> hasher;
  return hasher(*m_value);
}

} //namespace TransLucid

namespace std
{
  size_t
  hash<TransLucid::tuple_t>::operator()(const TransLucid::tuple_t& t) const
  {
    size_t h = 0;
    for(auto& v : t)
    {
      std::_Hash_impl::__hash_combine(v.first, h);
      std::_Hash_impl::__hash_combine(v.second, h);
    }

    return h;
  }
}
