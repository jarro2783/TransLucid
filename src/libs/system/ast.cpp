/* Abstract syntax tree.
   Copyright (C) 2009--2013 Jarryd Beck

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

/** @file ast.cpp
 * The AST implementation.
 */

#include <tl/ast.hpp>

namespace TransLucid
{

namespace Tree
{

LiteralExpr::LiteralExpr(const u32string& type, const u32string& text)
: type(type), text(text)
{
  rewritten =
    Tree::LambdaAppExpr( 
      Tree::LambdaAppExpr(Tree::IdentExpr(U"construct_literal"), type),
      text);
}

Expr
insert_binary_operator
(
  const BinaryOperator& op,
  Expr& lhs,
  Expr& rhs
)
{
  Tree::BinaryOpExpr* binop = get<Tree::BinaryOpExpr>(&lhs);
  if (binop == nullptr)
  {
    return Tree::BinaryOpExpr(op, lhs, rhs);
  }
  if (binop->op.precedence > op.precedence)
  {
    return Tree::BinaryOpExpr(op, lhs, rhs);
  }
  if (binop->op.precedence < op.precedence)
  {
    binop->add_right(op, rhs);
    return *binop;
  }
  //precedence is the same
  if (binop->op.assoc != op.assoc)
  {
    //error, mixed associativity of same precedence
    //TODO parse errors
    throw U"Mixed associativity of same precedence";
  }
  if (binop->op.assoc == ASSOC_NON)
  {
    //error multiple non assoc operators
    throw
      U"Multiple non associative operators of the same precedence";
  }
  if (binop->op.assoc == ASSOC_LEFT)
  {
    return Tree::BinaryOpExpr(op, lhs, rhs);
  }
  if (binop->op.assoc == ASSOC_RIGHT)
  {
    binop->add_right(op, rhs);
    return *binop;
  }
  //assoc_variable or assoc_comparison
  if (binop->op != op)
  {
    //error multiple variadic operators
    throw U"Multiple variadic operators of the same precedence";
  }
  //we have the same operator
  binop->add_leaf(rhs);
  return *binop;
}

void
BinaryOpExpr::add_right(const BinaryOperator& op, Expr& r)
{
  rhs = insert_binary_operator(op, rhs, r);
}

void
BinaryOpExpr::add_leaf(Expr& e)
{
  //operands.push_back(e);
  rhs = e;
}

} //namespace Tree

} //namespace TransLucid
