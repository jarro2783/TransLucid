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

#ifndef TL_TYPES_DETAIL_HPP
#define TL_TYPES_DETAIL_HPP

#include <tl/types.hpp>

namespace TransLucid
{
  namespace detail
  {
    template <>
    struct set_constant_func<bool>
    {
      void
      operator()(Constant& c, bool v)
      {
        c.data.tv = v;
        c.data.field = TYPE_FIELD_TV;
      }
    };

    template <>
    struct set_constant_func<Special>
    {
      void
      operator()(Constant& c, Special s)
      {
        c.data.sp = s;
        c.data.field = TYPE_FIELD_SP;
      }
    };

    template <>
    struct set_constant_func<int8_t>
    {
      void
      operator()(Constant& c, int8_t v)
      {
        c.data.si8 = v;
        c.data.field = TYPE_FIELD_SI8;
      }
    };

    template <>
    struct set_constant_func<uint16_t>
    {
      void
      operator()(Constant& c, uint16_t v)
      {
        c.data.ui16 = v;
        c.data.field = TYPE_FIELD_UI16;
      }
    };

    template <>
    struct get_constant_func<bool>
    {
      bool
      operator()(Constant& c)
      {
        return c.data.tv;
      }
    };

    template <>
    struct get_constant_func<int8_t>
    {
      int8_t
      operator()(Constant& c)
      {
        return c.data.si8;
      }
    };

    template <>
    struct get_constant_func<uint64_t>
    {
      uint64_t
      operator()(Constant& c)
      {
        return c.data.ui64;
      }
    };
  }
}

#endif
