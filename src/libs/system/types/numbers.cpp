/* The integer types.
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
#include <tl/system_util.hpp>
#include <tl/types/fixed_number.hpp>
#include <tl/types/function.hpp>
#include <tl/types/numbers.hpp>
#include <tl/types/string.hpp>

namespace TransLucid
{
namespace
{

FixedNumeric<int8_t> s8;
FixedNumeric<uint8_t> u8;
FixedNumeric<int16_t> s16;
FixedNumeric<uint16_t> u16;
FixedNumeric<int32_t> s32;
FixedNumeric<uint32_t> u32;
FixedNumeric<int64_t> s64;
FixedNumeric<uint64_t> u64;
FixedNumeric<float> f32;
FixedNumeric<double> f64;

}

void
registerIntegers(System& s)
{
  s8.init(s, U"sint8");
  u8.init(s, U"uint8");
  s16.init(s, U"sint16");
  u16.init(s, U"uint16");
  s32.init(s, U"sint32");
  u32.init(s, U"uint32");
  s64.init(s, U"sint64");
  u64.init(s, U"uint64");

  f32.init(s, U"float32");
  f64.init(s, U"float64");

  s.addDimension(U"prec");
  s.addDimension(U"is_signed");
}

}
