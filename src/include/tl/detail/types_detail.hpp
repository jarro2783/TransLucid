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
    template <typename T>
    struct clone;

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
    struct set_constant_func<char32_t>
    {
      void
      operator()(Constant& c, char32_t ch)
      {
        c.data.ch = ch;
        c.data.field = TYPE_FIELD_CH;
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
    struct set_constant_func<uint32_t>
    {
      void
      operator()(Constant& c, uint32_t v)
      {
        c.data.ui32 = v;
        c.data.field = TYPE_FIELD_UI32;
      }
    };

    template <>
    struct set_constant_func<int64_t>
    {
      void
      operator()(Constant& c, int64_t v)
      {
        c.data.si64 = v;
        c.data.field = TYPE_FIELD_SI64;
      }
    };

    template <>
    struct set_constant_func<uint64_t>
    {
      void
      operator()(Constant& c, uint64_t v)
      {
        c.data.ui64 = v;
        c.data.field = TYPE_FIELD_UI64;
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
    struct get_constant_func<Special>
    {
      Special
      operator()(const Constant& c) const
      {
        return c.data.sp;
      }
    };

    template <>
    struct get_constant_func<bool>
    {
      bool
      operator()(const Constant& c) const
      {
        return c.data.tv;
      }
    };

    template <>
    struct get_constant_func<char32_t>
    {
      char32_t
      operator()(const Constant& c) const
      {
        return c.data.ch;
      }
    };

    template <>
    struct get_constant_func<int8_t>
    {
      int8_t
      operator()(const Constant& c) const
      {
        return c.data.si8;
      }
    };

    template <>
    struct get_constant_func<uint8_t>
    {
      uint8_t
      operator()(const Constant& c) const
      {
        return c.data.ui8;
      }
    };

    template <>
    struct get_constant_func<int16_t>
    {
      int16_t
      operator()(const Constant& c) const
      {
        return c.data.si16;
      }
    };

    template <>
    struct get_constant_func<uint16_t>
    {
      uint16_t
      operator()(const Constant& c) const
      {
        return c.data.ui16;
      }
    };

    template <>
    struct get_constant_func<uint32_t>
    {
      uint32_t
      operator()(const Constant& c) const
      {
        return c.data.ui32;
      }
    };

    template <>
    struct get_constant_func<uint64_t>
    {
      uint64_t
      operator()(const Constant& c) const
      {
        return c.data.ui64;
      }
    };
  }
}

#endif