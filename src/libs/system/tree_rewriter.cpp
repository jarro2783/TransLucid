/* Rewrites concrete syntax
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

/** @file tree_rewriter.cpp
 * Rewrites an expression tree to remove operators, literals, and
 * call-by-name expressions.
 */

#include <tl/tree_rewriter.hpp>

namespace TransLucid
{

Tree::Expr
TreeRewriter::rewrite(const Tree::Expr& e)
{
  return apply_visitor(*this, e);
}

Tree::Expr 
TreeRewriter::operator()(const Tree::LiteralExpr& e)
{
  return 
    Tree::LambdaAppExpr( 
      Tree::LambdaAppExpr(Tree::IdentExpr(U"construct_literal"), e.type),
      e.text);
}

Tree::Expr 
TreeRewriter::operator()(const Tree::ParenExpr& e)
{
  //we can drop the parens now
  return apply_visitor(*this, e.e);
}

Tree::Expr 
TreeRewriter::operator()(const Tree::UnaryOpExpr& e)
{
  //(FN1 ! (#arg0)) @ [fnname <- e.op.op, arg0 <- T(e.e)]

  Tree::Expr expr = apply_visitor(*this, e.e);

  Tree::Expr result = Tree::LambdaAppExpr
  (
    Tree::IdentExpr(e.op.op),
    expr
  );

  return apply_visitor(*this, result);
}

Tree::Expr 
TreeRewriter::operator()(const Tree::BinaryOpExpr& e)
{
  //(e.op.op) . (e.lhs) . (e.rhs)
  //(FN2 ! (#arg0, #arg1))
  //  @ [fnname <- e.op.op, arg0 <- T(e.lhs), arg1 <- T(e.rhs)]

  Tree::Expr elhs = apply_visitor(*this, e.lhs);
  Tree::Expr erhs = apply_visitor(*this, e.rhs);

  Tree::Expr result;
    
  if (e.op.cbn)
  {
    result = Tree::PhiAppExpr
    (
      Tree::PhiAppExpr(Tree::IdentExpr(e.op.op), e.lhs),
      e.rhs
    );
  }
  else
  {
    result = Tree::LambdaAppExpr
    (
      Tree::LambdaAppExpr(Tree::IdentExpr(e.op.op), e.lhs),
      e.rhs
    );
  }

  return apply_visitor(*this, result);
}

Tree::Expr 
TreeRewriter::operator()(const Tree::BangAppExpr& e)
{
  //first check if it is #!E
  if (get<Tree::HashSymbol>(&e.name) != nullptr)
  {
    Tree::Expr rhs = apply_visitor(*this, e.args.at(0));
    return Tree::HashExpr(rhs);
  }
  else
  {
    Tree::Expr name = apply_visitor(*this, e.name);
    std::vector<Tree::Expr> args;

    for (auto expr : e.args)
    {
      args.push_back(apply_visitor(*this, expr));
    }

    return Tree::BangAppExpr(name, args);
  }
}

}
