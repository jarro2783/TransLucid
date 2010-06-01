/* Abstract syntax tree.
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

#include <tl/ast.hpp>

namespace TransLucid
{

namespace Tree
{

Expr
insert_binary_operator
(
  const BinaryOperator& op,
  Expr& lhs,
  Expr& rhs
)
{
  Tree::BinaryOpExpr* binop = boost::get<Tree::BinaryOpExpr>(&lhs);
  if (binop == 0)
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
    throw ParseError(U"Mixed associativity of same precedence");
  }
  if (binop->op.assoc == ASSOC_NON)
  {
    //error multiple non assoc operators
    throw
      ParseError(U"Multiple non associative operators of the same precedence");
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
    throw ParseError(U"Multiple variadic operators of the same precedence");
  }
  //we have the same operator
  binop->add_leaf(rhs);
  return *binop;
}

void
BinaryOpExpr::add_right(const BinaryOperator& op, Expr& r)
{
  //size_t last = operands.size()-1;
  //operands.at(last) =
  //  insert_binary_operator(op, operands.at(last), r);
  #warning hope this works rhs should be the last one when we dont have variadic
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