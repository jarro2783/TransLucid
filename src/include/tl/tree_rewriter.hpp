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

/**
 * @file tree_rewriter.hpp
 * Rewrites expression trees.
 */

#ifndef TL_TREE_REWRITER_HPP_INCLUDED
#define TL_TREE_REWRITER_HPP_INCLUDED

#include <tl/ast.hpp>

namespace TransLucid
{
  class TreeRewriter
  {
    public:
    typedef Tree::Expr result_type;

    Tree::Expr
    rewrite(const Tree::Expr& e);

    Tree::Expr operator()(const Tree::nil& n);
    Tree::Expr operator()(bool b);
    Tree::Expr operator()(Special s);
    Tree::Expr operator()(const mpz_class& i);
    Tree::Expr operator()(char32_t c);
    Tree::Expr operator()(const u32string& s);
    Tree::Expr operator()(const Tree::HashSymbol& e);
    Tree::Expr operator()(const Tree::LiteralExpr& e);
    Tree::Expr operator()(const Tree::DimensionExpr& e);
    Tree::Expr operator()(const Tree::IdentExpr& e);
    Tree::Expr operator()(const Tree::ParenExpr& e);
    Tree::Expr operator()(const Tree::UnaryOpExpr& e);
    Tree::Expr operator()(const Tree::BinaryOpExpr& e);
    Tree::Expr operator()(const Tree::IfExpr& e);
    Tree::Expr operator()(const Tree::HashExpr& e);
    Tree::Expr operator()(const Tree::TupleExpr& e);
    Tree::Expr operator()(const Tree::AtExpr& e);
    Tree::Expr operator()(const Tree::LambdaExpr& e);
    Tree::Expr operator()(const Tree::PhiExpr& e);
    Tree::Expr operator()(const Tree::BangAppExpr& e);
    Tree::Expr operator()(const Tree::LambdaAppExpr& e);
    Tree::Expr operator()(const Tree::PhiAppExpr& e);
    Tree::Expr operator()(const Tree::WhereExpr& e);
  };
}

#endif
