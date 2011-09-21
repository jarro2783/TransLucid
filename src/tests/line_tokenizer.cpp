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
#include <tl/output.hpp>

#define BOOST_TEST_MODULE LineTokenizer
#include <boost/test/included/unit_test.hpp>

namespace TL = TransLucid;

BOOST_AUTO_TEST_SUITE( line_tokenizer_tests )

BOOST_AUTO_TEST_CASE( empty )
{
  std::string input;

  TL::Parser::U32Iterator iter(
    TL::Parser::makeUTF8Iterator(input.begin()),
    TL::Parser::makeUTF8Iterator(input.end())
  );

  TL::LineTokenizer tokenize(iter);

  auto n = tokenize.next();

  BOOST_CHECK_EQUAL(n.first, TL::LineType::EMPTY);
  BOOST_CHECK_EQUAL(n.second, TL::u32string());
}

BOOST_AUTO_TEST_CASE( simple )
{
  std::string input = "eqn a = 5;; assign y := 6;;";

  TL::Parser::U32Iterator iter(
    TL::Parser::makeUTF8Iterator(input.begin()),
    TL::Parser::makeUTF8Iterator(input.end())
  );

  TL::LineTokenizer tokenize(iter);

  auto n = tokenize.next();
  BOOST_CHECK_EQUAL(n.first, TL::LineType::LINE);
  BOOST_CHECK_EQUAL(n.second, TL::u32string(U"eqn a = 5;;"));

  n = tokenize.next();
  BOOST_CHECK_EQUAL(n.first, TL::LineType::LINE);
  BOOST_CHECK_EQUAL(n.second, TL::u32string(U"assign y := 6;;"));
}

BOOST_AUTO_TEST_CASE( dollar )
{
  std::string input = "eqn a = b;;  $$";

  TL::Parser::U32Iterator iter(
    TL::Parser::makeUTF8Iterator(input.begin()),
    TL::Parser::makeUTF8Iterator(input.end())
  );

  TL::LineTokenizer tokenize(iter);

  auto n = tokenize.next();

  BOOST_CHECK_EQUAL(n.first, TL::LineType::LINE);
  BOOST_CHECK_EQUAL(n.second, TL::u32string(U"eqn a = b;;"));

  n = tokenize.next();
  BOOST_CHECK_EQUAL(n.first, TL::LineType::DOUBLE_DOLLAR);
  BOOST_CHECK_EQUAL(n.second, TL::u32string(U"$$"));

  n = tokenize.next();
  BOOST_CHECK_EQUAL(n.first, TL::LineType::EMPTY);
  BOOST_CHECK_EQUAL(n.second, TL::u32string());
}

BOOST_AUTO_TEST_CASE( extra_spaces )
{
  std::string input = "eqn a = b;;  $$  ";

  TL::Parser::U32Iterator iter(
    TL::Parser::makeUTF8Iterator(input.begin()),
    TL::Parser::makeUTF8Iterator(input.end())
  );

  TL::LineTokenizer tokenize(iter);

  auto n = tokenize.next();
  BOOST_CHECK_EQUAL(n.first, TL::LineType::LINE);
  BOOST_CHECK_EQUAL(n.second, TL::u32string(U"eqn a = b;;"));

  n = tokenize.next();
  BOOST_CHECK_EQUAL(n.first, TL::LineType::DOUBLE_DOLLAR);
  BOOST_CHECK_EQUAL(n.second, TL::u32string(U"$$"));

  n = tokenize.next();
  BOOST_CHECK_EQUAL(n.first, TL::LineType::EMPTY);
  BOOST_CHECK_EQUAL(n.second, TL::u32string());
}

BOOST_AUTO_TEST_CASE( percent )
{
  std::string input = "eqn x = 42;;\n%%\nx;;";

  TL::Parser::U32Iterator iter(
    TL::Parser::makeUTF8Iterator(input.begin()),
    TL::Parser::makeUTF8Iterator(input.end())
  );

  TL::LineTokenizer tokenize(iter);

  auto n = tokenize.next();
  BOOST_CHECK_EQUAL(n.first, TL::LineType::LINE);
  BOOST_CHECK_EQUAL(n.second, TL::u32string(U"eqn x = 42;;"));

  n = tokenize.next();
  BOOST_CHECK_EQUAL(n.first, TL::LineType::DOUBLE_PERCENT);
  BOOST_CHECK_EQUAL(n.second, TL::u32string(U"%%"));

  n = tokenize.next();
  BOOST_CHECK_EQUAL(n.first, TL::LineType::LINE);
  BOOST_CHECK_EQUAL(n.second, TL::u32string(U"x;;"));

  n = tokenize.next();
  BOOST_CHECK_EQUAL(n.first, TL::LineType::EMPTY);
  BOOST_CHECK_EQUAL(n.second, TL::u32string());
}

BOOST_AUTO_TEST_CASE( where )
{
  {
    std::string input = "x where var x = 5;; end;;";

    TL::Parser::U32Iterator iter(
      TL::Parser::makeUTF8Iterator(input.begin()),
      TL::Parser::makeUTF8Iterator(input.end())
    );

    TL::LineTokenizer tokenize(iter);

    auto n = tokenize.next();
    BOOST_CHECK_EQUAL(n.first, TL::LineType::LINE);
    BOOST_CHECK_EQUAL(n.second, TL::to_u32string(input));
  }

  {
    std::string input = 
R"(var f = x + #d where
   var x = 5;;
   dim d <- 4;;
end;;)";

    
    TL::Parser::U32Iterator iter(
      TL::Parser::makeUTF8Iterator(input.begin()),
      TL::Parser::makeUTF8Iterator(input.end())
    );

    TL::LineTokenizer tokenize(iter);

    auto n = tokenize.next();
    BOOST_CHECK_EQUAL(n.first, TL::LineType::LINE);
    BOOST_CHECK_EQUAL(n.second, TL::to_u32string(input));
  }

}

BOOST_AUTO_TEST_SUITE_END()
