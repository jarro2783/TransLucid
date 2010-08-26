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

#include <tl/types.hpp>
#include <boost/foreach.hpp>
//#include <boost/bind.hpp>
#include <tl/range.hpp>
#include <tl/system.hpp>
#include <tl/exception.hpp>
#include <tl/header.hpp>
#include <tl/footer.hpp>

namespace TransLucid
{

Tuple::Tuple()
: m_value(new tuple_t)
{
}

Tuple::Tuple(const tuple_t& tuple)
: m_value(new tuple_t(tuple))
{
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
  std::cerr << "printing tuple" << std::endl;
  os << "[";
  BOOST_FOREACH(const tuple_t::value_type& v, *m_value)
  {
    os << v.first << ":";
    v.second.print(os);
  }
  os << "]";
}

} //namespace TransLucid
