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

#include <boost/spirit/include/support_istream_iterator.hpp>

#define CATCH_CONFIG_MAIN
#include "catch.hpp"

namespace TL = TransLucid;

TEST_CASE ( "empty input", "empty input to line tokenizer" )
{
  std::string input;

  TL::Parser::U32Iterator iter(
    TL::Parser::makeUTF8Iterator(input.begin()),
    TL::Parser::makeUTF8Iterator(input.end())
  );

  TL::LineTokenizer tokenize(iter);

  auto n = tokenize.next();

  CHECK(n.first == TL::LineType::EMPTY);
  CHECK(n.second == TL::u32string());
}

TEST_CASE ( "simple", "simply line iterator tests" )
{
  std::string input = "eqn a = 5;; assign y := 6;;";

  TL::Parser::U32Iterator iter(
    TL::Parser::makeUTF8Iterator(input.begin()),
    TL::Parser::makeUTF8Iterator(input.end())
  );

  TL::LineTokenizer tokenize(iter);

  auto n = tokenize.next();
  CHECK(n.first == TL::LineType::LINE);
  CHECK(n.second == TL::u32string(U"eqn a = 5;;"));

  n = tokenize.next();
  CHECK(n.first == TL::LineType::LINE);
  CHECK(n.second == TL::u32string(U"assign y := 6;;"));
}

TEST_CASE( "dollar symbol", "line tokenizer $$" )
{
  std::string input = "eqn a = b;;  $$";

  TL::Parser::U32Iterator iter(
    TL::Parser::makeUTF8Iterator(input.begin()),
    TL::Parser::makeUTF8Iterator(input.end())
  );

  TL::LineTokenizer tokenize(iter);

  auto n = tokenize.next();

  CHECK(n.first == TL::LineType::LINE);
  CHECK(n.second == TL::u32string(U"eqn a = b;;"));

  n = tokenize.next();
  CHECK(n.first == TL::LineType::DOUBLE_DOLLAR);
  CHECK(n.second == TL::u32string(U"$$"));

  n = tokenize.next();
  CHECK(n.first == TL::LineType::EMPTY);
  CHECK(n.second == TL::u32string());
}

TEST_CASE( "white space", "line tokenizer white space" )
{
  std::string input = "eqn a = b;;  $$  ";

  TL::Parser::U32Iterator iter(
    TL::Parser::makeUTF8Iterator(input.begin()),
    TL::Parser::makeUTF8Iterator(input.end())
  );

  TL::LineTokenizer tokenize(iter);

  auto n = tokenize.next();
  CHECK(n.first == TL::LineType::LINE);
  CHECK(n.second == TL::u32string(U"eqn a = b;;"));

  n = tokenize.next();
  CHECK(n.first == TL::LineType::DOUBLE_DOLLAR);
  CHECK(n.second == TL::u32string(U"$$"));

  n = tokenize.next();
  CHECK(n.first == TL::LineType::EMPTY);
  CHECK(n.second == TL::u32string());
}

TEST_CASE( "%%", "line tokenizer %%" )
{
  std::string input = "eqn x = 42;;\n%%\nx;;";

  TL::Parser::U32Iterator iter(
    TL::Parser::makeUTF8Iterator(input.begin()),
    TL::Parser::makeUTF8Iterator(input.end())
  );

  TL::LineTokenizer tokenize(iter);

  auto n = tokenize.next();
  CHECK(n.first == TL::LineType::LINE);
  CHECK(n.second == TL::u32string(U"eqn x = 42;;"));

  n = tokenize.next();
  CHECK(n.first == TL::LineType::DOUBLE_PERCENT);
  CHECK(n.second == TL::u32string(U"%%"));

  n = tokenize.next();
  CHECK(n.first == TL::LineType::LINE);
  CHECK(n.second == TL::u32string(U"x;;"));

  n = tokenize.next();
  CHECK(n.first == TL::LineType::EMPTY);
  CHECK(n.second == TL::u32string());
}

TEST_CASE( "where clause", "does a where clause with vars work" )
{
  {
    std::string input = "x where var x = 5;; end;;";

    TL::Parser::U32Iterator iter(
      TL::Parser::makeUTF8Iterator(input.begin()),
      TL::Parser::makeUTF8Iterator(input.end())
    );

    TL::LineTokenizer tokenize(iter);

    auto n = tokenize.next();
    CHECK(n.first == TL::LineType::LINE);
    CHECK(n.second == TL::to_u32string(input));
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
    CHECK(n.first == TL::LineType::LINE);
    CHECK(n.second == TL::to_u32string(input));
  }

}

TEST_CASE ("istringstream", "input in an istringstream")
{
  std::istringstream is("var f = 42;;\n%%\n5;;\nf;;");
  is >> std::noskipws;

  TL::Parser::U32Iterator begin(
    TL::Parser::makeUTF8Iterator(boost::spirit::istream_iterator(is)),
    TL::Parser::makeUTF8Iterator(boost::spirit::istream_iterator())
  );

  TL::LineTokenizer tok(begin);
  auto n = tok.next();
  CHECK(n.first == TL::LineType::LINE);
  CHECK(n.second == U"var f = 42;;");

  n = tok.next();
  CHECK(n.first == TL::LineType::DOUBLE_PERCENT);

  n = tok.next();
  CHECK(n.first == TL::LineType::LINE);
  CHECK(n.second == U"5;;");
}
