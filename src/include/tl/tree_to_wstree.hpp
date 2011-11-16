/* Simplifies the Expr trees.
   Copyright (C) 2011 Jarryd Beck

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
 * @file tree_to_wstree.hpp
 * Rewrites expression trees.
 */

#ifndef TL_TREE_TO_WSTREE
#define TL_TREE_TO_WSTREE

#include <tl/ast.hpp>

namespace TransLucid
{
  class System;

  class TreeToWSTree
  {
    private:

    System *m_system;

    public:
    typedef Tree::Expr result_type;

    TreeToWSTree(System* system)
    : m_system(system)
    {
    }

    Tree::Expr
    toWSTree(const Tree::Expr& expr);
    
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

    const std::vector<Parser::Equation>& newVars() const
    {
      return m_newVars;
    }

    const std::vector<dimension_index>& getLin() const
    {
      return m_Lin;
    }

    const std::vector<dimension_index>& getAllScopeArgs() const
    {
      return m_namedAllScopeArgs;
    }

    const std::vector<dimension_index>& getAllScopeOdometer() const
    {
      return m_namedAllScopeOdometers;
    }

    private:

    Tree::WhereExpr 
    renameWhereExpr(const Tree::WhereExpr& e);

    std::vector<dimension_index> m_Lout;
    std::vector<dimension_index> m_Lin;

    std::vector<Parser::Equation> m_newVars;

    //we actually only need one scope stack
    std::vector<dimension_index> m_scope;

    std::vector<dimension_index> m_namedAllScopeArgs;
    std::vector<dimension_index> m_namedAllScopeOdometers;
  };
}

#endif
