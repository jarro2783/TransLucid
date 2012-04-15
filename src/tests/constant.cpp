/* Constant tests.
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

#include <tl/types.hpp>
#include <tl/fixed_indexes.hpp>

#define CATCH_CONFIG_MAIN
#include "catch.hpp"

namespace TL = TransLucid;

template <typename T>
void
test_constant(T&& v, TL::type_index ti)
{
  TL::Constant c(std::forward<T>(v), ti);

  CHECK(c.index() == ti);
  CHECK(v == TL::get_constant<T>(c));
}

void
test_bool()
{
  TL::type_index ti = TL::TYPE_INDEX_BOOL;
  test_constant(true, ti);
  test_constant(false, ti);
}

TEST_CASE ( "get set", "get_constant and set_constant work for all types" )
{
  test_bool();
}
