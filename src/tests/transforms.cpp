/* Cache tests.
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

/**
 * @file transforms.cpp
 * Transformation tests.
 */

#include <tl/free_variables.hpp>
#include <tl/output.hpp>
#include <tl/system.hpp>
#include <tl/tree_printer.hpp>
#include <tl/utility.hpp>

#define CATCH_CONFIG_MAIN
#include "catch.hpp"

namespace TL = TransLucid;

TL::Tree::Expr
parseExpression(TL::System& s, const TL::u32string& input)
{
  TL::Parser::U32Iterator lineBegin(
    TL::Parser::makeUTF32Iterator(input.begin()));
  TL::Parser::U32Iterator lineEnd(
    TL::Parser::makeUTF32Iterator(input.end())
  );

  TL::Parser::StreamPosIterator posbegin(lineBegin, input,
    0,0);
  TL::Parser::StreamPosIterator posend(lineEnd);

  TL::Tree::Expr result;
  REQUIRE(s.parseExpression(posbegin, posend, result));

  return result;
}

void
testFreeVars
(
  const TL::u32string& input,
  std::set<TL::u32string> expected
)
{
  TL::System s;

  TL::Tree::Expr e1 = parseExpression(s, input);

  TL::FreeVariableReplacer free(s);

  TL::Tree::Expr e2 = free.replaceFree(e1);

  const TL::Tree::LambdaExpr& lambda = TL::get<TL::Tree::LambdaExpr>(e2);

  const auto& replaced = lambda.free;
  auto current = replaced.begin();

  auto expect = expected.begin();
  while (expect != expected.end())
  {
    REQUIRE(current != replaced.end());
    CHECK(current->first == *expect);
    ++expect;
    ++current;
  }

  REQUIRE(current == replaced.end());

  auto fixed = TL::fixupTree(s, e1);
  std::cerr << TL::Printer::print_expr_tree(fixed.first) << std::endl;
}

TEST_CASE ( "factorial", "free variables in factorial function" )
{
  testFreeVars
  (
    UR"(
    \n -> f where
      dim d <- n;;
      var f [d : 0] = 1;;
      var f [d : pos] = times.(#!d).(prev.d f);;
    end;;
    )",
    {
      U"pos", U"prev", U"times"
    }
  );

  testFreeVars
  (
    UR"raw(\c ->
  plus.(plus.(plus.(print_typename.c)."\"").(escape_string.(print.c)))."\""
    )raw",
    {
      U"escape_string", U"plus", U"print_typename", U"print"
    }
  );

}
