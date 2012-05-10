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
}

Tree::Expr 
TreeRewriter::operator()(const Tree::LiteralExpr& e)
{
}

Tree::Expr 
TreeRewriter::operator()(const Tree::DimensionExpr& e)
{
}

Tree::Expr 
TreeRewriter::operator()(const Tree::IdentExpr& e)
{
}

Tree::Expr 
TreeRewriter::operator()(const Tree::ParenExpr& e)
{
}

Tree::Expr 
TreeRewriter::operator()(const Tree::UnaryOpExpr& e)
{
}

Tree::Expr 
TreeRewriter::operator()(const Tree::BinaryOpExpr& e)
{
}

Tree::Expr 
TreeRewriter::operator()(const Tree::IfExpr& e)
{
}

Tree::Expr 
TreeRewriter::operator()(const Tree::HashExpr& e)
{
}

Tree::Expr 
TreeRewriter::operator()(const Tree::TupleExpr& e)
{
}

Tree::Expr 
TreeRewriter::operator()(const Tree::AtExpr& e)
{
}

Tree::Expr 
TreeRewriter::operator()(const Tree::LambdaExpr& e)
{
}

Tree::Expr 
TreeRewriter::operator()(const Tree::PhiExpr& e)
{
}

Tree::Expr 
TreeRewriter::operator()(const Tree::BangAppExpr& e)
{
}

Tree::Expr 
TreeRewriter::operator()(const Tree::LambdaAppExpr& e)
{
}

Tree::Expr 
TreeRewriter::operator()(const Tree::PhiAppExpr& e)
{
}

Tree::Expr 
TreeRewriter::operator()(const Tree::WhereExpr& e)
{
}


}
