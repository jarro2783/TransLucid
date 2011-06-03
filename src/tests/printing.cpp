/* Printing tests.
   Copyright (C) 2009, 2010 Jarryd Beck and John Plaice

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

#define BOOST_TEST_MODULE expressions
#include <boost/test/included/unit_test.hpp>

#include <tl/tree_printer.hpp>
#include <tl/system.hpp>
#include <tl/ast.hpp>

using namespace TransLucid;

namespace
{
SystemHD tlsystem;
std::string generated;
}

BOOST_AUTO_TEST_CASE ( integer )
{
  Tree::Expr ast = mpz_class(42);

  generated.clear();
  generated = print_expr_tree(ast);
  //BOOST_CHECK(Printer::karma::generate(outit, print_grammar, ast));

  BOOST_CHECK_EQUAL(generated, "42");

  ast = Tree::BinaryOpExpr
  (
    Tree::BinaryOperator
    (
      Tree::ASSOC_LEFT,
      U"operator+",
      U"+",
      5
    ),
   mpz_class(10),
   mpz_class(15)
  );

  //generated.clear();
  //BOOST_CHECK(Printer::karma::generate(outit, print_grammar, ast));
  generated = print_expr_tree(ast);
  BOOST_CHECK_EQUAL(generated, "(10+15)");
}

BOOST_AUTO_TEST_CASE ( tuple )
{
  u32string input = U"[1:1]";
  tlsystem.translate_expr(input);

  generated.clear();
  generated = print_expr_tree(tlsystem.lastExpression());

  BOOST_CHECK_EQUAL(generated, "[1:1]");
}

BOOST_AUTO_TEST_CASE ( hash_expr )
{
  u32string input(U"#1");
  tlsystem.translate_expr(input);

  generated.clear();
  generated = print_expr_tree(tlsystem.lastExpression());

  BOOST_CHECK_EQUAL(generated, "(#1)");
}

BOOST_AUTO_TEST_CASE ( at )
{
  u32string input(U"#1 @ [1:2]");
  tlsystem.translate_expr(input);

  generated.clear();
  generated = print_expr_tree(tlsystem.lastExpression());

  BOOST_CHECK_EQUAL(generated, "((#1)@[1:2])");
}
