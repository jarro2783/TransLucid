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
#include <tl/types/integer.hpp>
#include <tl/types/string.hpp>

namespace TransLucid
{
namespace
{

FixedInteger<int8_t> s8;

}

void
registerIntegers(System& s)
{
  s8.init(s, U"int8");
}

}
