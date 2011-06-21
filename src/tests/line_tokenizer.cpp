/* Line tokenizer tests.
   Copyright (C) 2011 Jarryd Beck

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

#include <tl/line_tokenizer.hpp>

#define BOOST_TEST_MODULE LineTokenizer
#include <boost/test/included/unit_test.hpp>

namespace TL = TransLucid;

BOOST_AUTO_TEST_SUITE( line_tokenizer_tests )

BOOST_AUTO_TEST_CASE( simple )
{
  std::string input = "eqn a = 5;; assign y := 6;;";

  TL::Parser::U32Iterator iter(
    TL::Parser::makeUTF8Iterator(input.begin()),
    TL::Parser::makeUTF8Iterator(input.end())
  );

  TL::LineTokenizer tokenize(iter);

  TL::u32string n = tokenize.next();
  BOOST_CHECK_EQUAL(n, TL::u32string(U"eqn a = 5;;"));

  n = tokenize.next();
  BOOST_CHECK_EQUAL(n, TL::u32string(U" assign y := 6;;"));
}

BOOST_AUTO_TEST_SUITE_END()
