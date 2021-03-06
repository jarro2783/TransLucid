/* Set values.
   Copyright (C) 2009, 2010 Jarryd Beck

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

#include <tl/set_types.hpp>

namespace TransLucid
{

TypeAsSet::TypeAsSet(size_t index)
: m_index(index)
{
}

bool
TypeAsSet::is_member(const Constant& v)
{
  return v.index() == m_index;
}

//is s a subset of this
bool
TypeAsSet::is_subset(const SetBase& s)
{
}

}
