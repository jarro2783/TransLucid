/* Set values.
   Copyright (C) 2009--2013 Jarryd Beck

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

#ifndef SET_TYPES_HPP_INCLUDED
#define SET_TYPES_HPP_INCLUDED

#include <tl/builtin_types.hpp>
#include <tl/workshop.hpp>

namespace TransLucid
{
  class TypeAsSet : public SetBase
  {
    public:
    TypeAsSet(size_t index);

    bool
    is_member(const Constant& v);

    //is s a subset of this
    bool
    is_subset(const SetBase& s);

    private:
    size_t m_index;
  };
}

#endif // SET_TYPES_HPP_INCLUDED
