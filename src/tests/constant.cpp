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

#include <tl/range.hpp>
#include <tl/types.hpp>
#include <tl/fixed_indexes.hpp>

#include <limits>
#include <gmpxx.h>

#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#define TI_UI8 (TL::TYPE_INDEX_LAST + 1)
#define TI_SI8 (TL::TYPE_INDEX_LAST + 2)
#define TI_SI16 (TL::TYPE_INDEX_LAST + 3)
#define TI_UI16 (TL::TYPE_INDEX_LAST + 4)
#define TI_SI32 (TL::TYPE_INDEX_LAST + 5)
#define TI_UI32 (TL::TYPE_INDEX_LAST + 6)
#define TI_SI64 (TL::TYPE_INDEX_LAST + 7)
#define TI_UI64 (TL::TYPE_INDEX_LAST + 8)
#define TI_FLOAT (TL::TYPE_INDEX_LAST + 9)
#define TI_DOUBLE (TL::TYPE_INDEX_LAST + 10)

namespace TL = TransLucid;

template <typename T>
void
test_get_constant(T&& v, TL::type_index ti)
{
  TL::Constant c(std::forward<T>(v), ti);

  CHECK(c.index() == ti);
  CHECK(v == TL::get_constant<typename std::remove_reference<T>::type>(c));
}

void
test_get_bool()
{
  TL::type_index ti = TL::TYPE_INDEX_BOOL;
  test_get_constant(true, ti);
  test_get_constant(false, ti);
}

template <typename T>
void
test_get_integer(TL::type_index ti)
{
  //test 100 values in the range of the type

  std::numeric_limits<T> limits;

  T min = limits.min();
  T max = limits.max();
  T step = 1;

  if (limits.is_signed)
  {
    step = max / 50;
  }
  else
  {
    step = max / 100;
  }

  int i = 0;
  T value = min;
  while (i != 100)
  {
    test_get_constant(value, ti);
    value += step;
    ++i;
  }
}

void
test_get_character()
{
  //at least test ascii
  char32_t c = 1;

  for (; c != 0x7F; ++c)
  {
    test_get_constant(c, TL::TYPE_INDEX_UCHAR);
  }

  //then test bigger ones to make sure they can be stored
  test_get_constant(0xA1, TL::TYPE_INDEX_UCHAR);
  test_get_constant(0xA1, TL::TYPE_INDEX_UCHAR);
  test_get_constant(0xA1, TL::TYPE_INDEX_UCHAR);
  test_get_constant(0x1A6, TL::TYPE_INDEX_UCHAR);
  test_get_constant(0x800, TL::TYPE_INDEX_UCHAR);
  test_get_constant(0x10AC, TL::TYPE_INDEX_UCHAR);
  test_get_constant(0x10400, TL::TYPE_INDEX_UCHAR);
  test_get_constant(0x100518, TL::TYPE_INDEX_UCHAR);
}

template <typename T>
void
test_get_float(TL::type_index ti)
{
  test_get_constant<T>(1, ti);
  test_get_constant<T>(1.5, ti);
  test_get_constant<T>(2.5, ti);
  test_get_constant<T>(100.6, ti);
}

TEST_CASE ( "get set", "get_constant and set_constant work for all types" )
{
  test_get_bool();

  test_get_integer<int8_t>(TI_UI8);
  test_get_integer<uint8_t>(TI_SI8);
  test_get_integer<int16_t>(TI_UI16);
  test_get_integer<uint16_t>(TI_SI16);
  test_get_integer<int32_t>(TI_UI32);
  test_get_integer<uint32_t>(TI_SI32);
  test_get_integer<int64_t>(TI_UI64);
  test_get_integer<uint64_t>(TI_SI64);

  test_get_character();

  test_get_float<float>(TI_FLOAT);
  test_get_float<double>(TI_DOUBLE);
}

TEST_CASE ( "equality", "constant equality" )
{
}

TEST_CASE ( "range", "range tests" )
{
  mpz_class a(1);
  mpz_class b(9);

  TL::Range r1(&a, &b);

  mpz_class c(5);
  mpz_class d(15);

  TL::Range r2(&c, &d);

  CHECK(r1.overlaps(r2));

  TL::Range r3 = r1.join(r2);

  REQUIRE(r3.lower() != nullptr);
  CHECK(*r3.lower() == 1);

  REQUIRE(r3.upper() != nullptr);
  CHECK(*r3.upper() == 15);
}
