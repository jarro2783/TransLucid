/* Parser tests.
   Copyright (C) 2009, 2010, 2011 Jarryd Beck and John Plaice

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

#undef DISABLE_CATCH

#include "tl/parser-new.hpp"
#include <tl/tree_printer.hpp>

#ifndef DISABLE_CATCH
#define CATCH_CONFIG_MAIN
#include "catch.hpp"
#endif

#ifdef DISABLE_CATCH
#define CHECK(x)
#endif

namespace TL = TransLucid;

#ifndef DISABLE_CATCH
TEST_CASE( "lexer iterator", "the lexer token stream iterator" )
#else
void test2()
#endif
{
  TL::System s;

  std::string input("42");

  TL::Parser::StreamPosIterator rawbegin
  (
    TL::Parser::U32Iterator(
      TL::Parser::makeUTF8Iterator(input.begin()),
      TL::Parser::makeUTF8Iterator(input.end())
    ),
    U"42"
  );

  TL::Parser::StreamPosIterator rawend{TL::Parser::U32Iterator()};

  TL::Parser::LexerIterator begin
  (
    rawbegin,
    rawend,
    s.getDefaultContext(),
    s.lookupIdentifiers()
  );

  TL::Parser::LexerIterator other = begin;

  TL::Parser::LexerIterator end = begin.makeEnd();

  CHECK(begin->getType() == TL::Parser::TOKEN_INTEGER);
  ++begin;
  CHECK(*begin == 0);

  CHECK(begin == end);

  ++other;
  CHECK(other == end);
}

#ifndef DISABLE_CATCH
TEST_CASE( "expr parser", "basic expression parser tests" )
#else
void test3()
#endif
{
  TL::System s;

  TL::Parser::Parser p(s);

  std::string input("42");

  TL::Parser::StreamPosIterator rawbegin
  (
    TL::Parser::U32Iterator(
      TL::Parser::makeUTF8Iterator(input.begin()),
      TL::Parser::makeUTF8Iterator(input.end())
    ),
    U"42"
  );

  TL::Parser::StreamPosIterator rawend{TL::Parser::U32Iterator()};

  TL::Parser::LexerIterator begin
  (
    rawbegin,
    rawend,
    s.getDefaultContext(),
    s.lookupIdentifiers()
  );

  TL::Parser::LexerIterator end = begin.makeEnd();

  TL::Tree::Expr result;
  bool success = p.parse_expr(begin, end, result);

  CHECK(success);
  CHECK(begin == end);
  CHECK(TL::get<mpz_class>(result) == 42);

  //function application
  std::string input2("f.d A B");

  TL::Parser::StreamPosIterator rawbegin2
  (
    TL::Parser::U32Iterator(
      TL::Parser::makeUTF8Iterator(input2.begin()),
      TL::Parser::makeUTF8Iterator(input2.end())
    ),
    U"f.d A B"
  );

  TL::Parser::LexerIterator begin2
  {
    rawbegin2,
    rawend,
    s.getDefaultContext(),
    s.lookupIdentifiers()
  };
  TL::Parser::LexerIterator end2 = begin2.makeEnd();

  CHECK(p.parse_expr(begin2, end2, result));
  CHECK(print_expr_tree(result) == "f.d A B");

  TL::Tree::PhiAppExpr* appB = TL::get<TL::Tree::PhiAppExpr>(&result);
  REQUIRE(appB != 0);

  TL::Tree::PhiAppExpr* appA = TL::get<TL::Tree::PhiAppExpr>(&appB->lhs);
  REQUIRE(appA != 0);

  TL::Tree::LambdaAppExpr* appd = TL::get<TL::Tree::LambdaAppExpr>
    (&appA->lhs);
  REQUIRE(appd != 0);
}

#ifdef DISABLE_CATCH
int main()
{
  test2();
  test3();
  return 0;
}
#endif
