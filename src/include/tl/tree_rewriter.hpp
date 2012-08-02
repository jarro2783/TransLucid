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
#include <tl/generic_walker.hpp>

namespace TransLucid
{
  class TreeRewriter : private GenericTreeWalker<TreeRewriter>
  {
    public:
    typedef Tree::Expr result_type;

    using GenericTreeWalker::operator();

    Tree::Expr
    rewrite(const Tree::Expr& e);

    Tree::Expr operator()(const Tree::LiteralExpr& e);
    Tree::Expr operator()(const Tree::ParenExpr& e);
    Tree::Expr operator()(const Tree::UnaryOpExpr& e);
    Tree::Expr operator()(const Tree::BinaryOpExpr& e);
    Tree::Expr operator()(const Tree::BangAppExpr& e);
  };
}

#endif
