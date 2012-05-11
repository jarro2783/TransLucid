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
 * Rewrites an expression tree to remove operators and literals.
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
TreeRewriter::operator()(const Tree::nil& n)
{
  return n;
}

Tree::Expr 
TreeRewriter::operator()(bool b)
{
  return b;
}

Tree::Expr 
TreeRewriter::operator()(Special s)
{
  return s;
}

Tree::Expr 
TreeRewriter::operator()(const mpz_class& i)
{
  return i;
}

Tree::Expr 
TreeRewriter::operator()(char32_t c)
{
  return c;
}

Tree::Expr 
TreeRewriter::operator()(const u32string& s)
{
  return s;
}

Tree::Expr 
TreeRewriter::operator()(const Tree::HashSymbol& e)
{
  return e;
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
TreeRewriter::operator()(const Tree::DimensionExpr& e)
{
  return e;
}

Tree::Expr 
TreeRewriter::operator()(const Tree::IdentExpr& e)
{
  return e;
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
TreeRewriter::operator()(const Tree::IfExpr& e)
{
  //do the elseifs
  std::vector<std::pair<Tree::Expr, Tree::Expr>> else_ifs;
  for (auto p : e.else_ifs)
  {
    else_ifs.push_back(std::make_pair(
      apply_visitor(*this, p.first),
      apply_visitor(*this, p.second)
    ));
  }

  return Tree::IfExpr(
    apply_visitor(*this, e.condition),
    apply_visitor(*this, e.then),
    else_ifs,
    apply_visitor(*this, e.else_)
  );
}

Tree::Expr 
TreeRewriter::operator()(const Tree::HashExpr& e)
{
  return Tree::HashExpr(apply_visitor(*this, e.e), e.cached);
}

Tree::Expr 
TreeRewriter::operator()(const Tree::TupleExpr& e)
{
  std::vector<std::pair<Tree::Expr, Tree::Expr>> tuple;
  for (auto p : e.pairs)
  {
    tuple.push_back(std::make_pair(
      apply_visitor(*this, p.first),
      apply_visitor(*this, p.second)
    ));
  }

  return Tree::TupleExpr(tuple);
}

Tree::Expr 
TreeRewriter::operator()(const Tree::AtExpr& e)
{
  return Tree::AtExpr(
    apply_visitor(*this, e.lhs),
    apply_visitor(*this, e.rhs)
  );
}

Tree::Expr 
TreeRewriter::operator()(const Tree::LambdaExpr& e)
{
  return Tree::LambdaExpr(e.name, apply_visitor(*this, e.rhs));
}

Tree::Expr 
TreeRewriter::operator()(const Tree::PhiExpr& e)
{
  return Tree::PhiExpr(e.name, apply_visitor(*this, e.rhs));
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

Tree::Expr 
TreeRewriter::operator()(const Tree::LambdaAppExpr& e)
{
  return Tree::LambdaAppExpr
  (
    apply_visitor(*this, e.lhs),
    apply_visitor(*this, e.rhs)
  );
}

Tree::Expr 
TreeRewriter::operator()(const Tree::PhiAppExpr& e)
{
  return Tree::PhiAppExpr
  (
    apply_visitor(*this, e.lhs),
    apply_visitor(*this, e.rhs)
  );
}

Tree::Expr 
TreeRewriter::operator()(const Tree::WhereExpr& e)
{
  Tree::WhereExpr where;

  where.e = e.e;

  //replace everything in the dimension expressions
  for (const auto& dim : e.dims)
  {
    where.dims.push_back(std::make_pair(dim.first,
      apply_visitor(*this, dim.second)));
  }

  //replace everything in the var expressions
  for (const auto& var : e.vars)
  {
    where.vars.push_back(std::make_tuple(
      std::get<0>(var),
      apply_visitor(*this, std::get<1>(var)),
      apply_visitor(*this, std::get<2>(var)),
      apply_visitor(*this, std::get<3>(var))
    ));
  }

  return where;
}


}
