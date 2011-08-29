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

/**
 * @file types_fwd.hpp
 * Forward declarations for types.
 */

#ifndef TL_TYPES_FWD_HPP
#define TL_TYPES_FWD_HPP

namespace TransLucid
{
  class Constant;

  namespace detail
  {
    /**
     * Given a data type that a Constant can hold in its union, sets the
     * value.
     */
    template <typename t>
    struct set_constant_func;

    /**
     * Given a data type that a Constant can hold in its union, gets the
     * value.
     */
    template <typename t>
    struct get_constant_func;

    /**
     * Templated function for turning any value into a pointer type.
     */
    //template <typename T>
    //struct make_constant_pointer;

    /**
     * Tests two constants for equality. Compares the appropriate
     * field of the union for equality.
     * @pre lhs.index == rhs.index and they don't use the
     * ptr field.
     */
    bool
    constant_equality(const Constant& lhs, const Constant& rhs);
  }
}

#endif
