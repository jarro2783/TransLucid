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

#include <iostream>
#include <sstream>

#include <tl/fixed_indexes.hpp>
#include <tl/types.hpp>
#include <tl/types/intmp.hpp>
#include <tl/range.hpp>
//#include <tl/system.hpp>
#include <tl/exception.hpp>
#include <gmpxx.h>

namespace TransLucid
{

namespace 
{

std::shared_ptr<tuple_t> sharedEmpty(new tuple_t);

template <typename T>
bool
equality(const Constant& lhs, const Constant& rhs)
{
  return get_constant<T>(lhs) == get_constant<T>(rhs);
}

bool
equality_true(const Constant& lhs, const Constant& rhs)
{
  return true;
}

template <typename T>
bool
less(const Constant& lhs, const Constant& rhs)
{
  return get_constant<T>(lhs) < get_constant<T>(rhs);
}

bool
less_false(const Constant& lhs, const Constant& rhs)
{
  return false;
}

template <typename T>
size_t
hash_func(const Constant& c)
{
  return std::hash<T>()(get_constant<T>(c));
}

size_t
hash_zero(const Constant& c)
{
  return 0;
}

bool (*equality_functions[TYPE_FIELD_PTR])(const Constant&, const Constant&) = 
{ 
  &equality_true,
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

bool (*less_functions[TYPE_FIELD_PTR])(const Constant&, const Constant&) = 
{ 
  &less_false, //because all errors values are equal
  &less<Special>,
  &less<bool>,
  &less<char32_t>,
  &less<int8_t>,
  &less<uint8_t>,
  &less<int16_t>,
  &less<uint16_t>,
  &less<int32_t>,
  &less<uint32_t>,
  &less<int64_t>,
  &less<uint64_t>,
  &less<float>,
  &less<double>
};

size_t (*hash_functions[TYPE_FIELD_PTR])(const Constant&) =
{
  &hash_zero,
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

bool
constant_less(const Constant& lhs, const Constant& rhs)
{
  return (*less_functions[lhs.data.field])(lhs, rhs);
}

}

Tuple::Tuple()
: m_value(sharedEmpty)
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

bool
Tuple::operator<(const Tuple& rhs) const
{
  auto iterl = begin();
  auto iterr = rhs.begin();

  while (iterl != end() && iterr != rhs.end())
  {
    if (iterl->first < iterr->first)
    {
      return true;
    }
    else if (iterr->first < iterl->first)
    {
      return false;
    }
    else if (iterl->second < iterr->second)
    {
      return true;
    }
    else if (iterr->second < iterl->second)
    {
      return false;
    }
    //this pair is equal, go to the next one
    ++iterl;
    ++iterr;
  }

  if (iterl == end() && iterr == rhs.end())
  {
    //they must be equal, so return false
    return false;
  }
  else if (iterl == end())
  {
    return true;
  }
  else
  {
    return false;
  }
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

std::string
print_constant(const Constant& c)
{
  std::ostringstream str;
  switch (c.index())
  {
    case TYPE_INDEX_DIMENSION:
    str << "dimension: " << get_constant<dimension_index>(c);
    return str.str();
    break;

    case TYPE_INDEX_INTMP:
    return Types::Intmp::get(c).get_str();
    break;

    case TYPE_INDEX_CALC:
    return "calc";
    break;

    case TYPE_INDEX_DEMAND:
    return "demand";
    break;

    case TYPE_INDEX_INTENSION:
    return "intension";
    break;

    default:
    str << "unknown of index " << c.index();
    return str.str();
    break;
  }
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
