/* Forward declaration for types.
   Copyright (C) 2011 Jarryd Beck and John Plaice

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

#ifndef TL_TYPES_FWD_HPP
#define TL_TYPES_FWD_HPP

namespace TransLucid
{
  class Constant;

  namespace detail
  {
    template <typename t>
    struct set_constant_func;

    template <typename t>
    struct get_constant_func;

    //precondition, lhs.index == rhs.index and they don't use the
    //ptr field
    bool
    constant_equality(const Constant& lhs, const Constant& rhs);
  }
}

#endif
