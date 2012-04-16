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
#include <tl/types/function.hpp>
#include <tl/types/integer.hpp>
#include <tl/types/string.hpp>

#include <sstream>

namespace TransLucid
{

namespace
{

template <typename T>
class FixedInteger
{
  public:

  void
  init(System& s, const u32string& name)
  {
    index = s.getTypeIndex(name);
  }

  Constant
  construct(const Constant& c)
  {
    T value;

    if (c.index() != TYPE_INDEX_USTRING)
    {
      return Types::Special::create(SP_UNDEF);
    }

    u32string& s = Types::String::get(c);

    std::istringstream is(s);
    is >> value;

    return Constant(value, index);
  }

  type_index index;
};

FixedInteger<int8_t> s8;

}

void
registerIntegers(System& s)
{
  s8.init(s, U"int8");
}

}
