/* Rewrites trees according to the semantics transformation
   Copyright (C) 2011,2012 Jarryd Beck

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
 * @file semantic_transform.hpp
 * Transforms expression trees according to the semantics in the paper:
 * Higher-order multidimensional programming
 * The input tree must:
 * 1. Have names renamed so that they are unique,
 * 2. Have free variables marked in functions
 *
 * The output is a tree rewritten according to the transformation. Any extra
 * equations added by where clauses and functions are recorded in a list.
 */

#ifndef TL_SEMANTIC_TRANSFORM_HPP_INCLUDED
#define TL_SEMANTIC_TRANSFORM_HPP_INCLUDED

#include <tl/ast.hpp>
#include <tl/generic_walker.hpp>

namespace TransLucid
{
  class System;

  class SemanticTransform : public GenericTreeWalker<SemanticTransform>
  {
    public:
    typedef Tree::Expr result_type;

    using GenericTreeWalker::operator();

    SemanticTransform(System& system)
    : m_system(system)
    {
    }

    Tree::Expr
    transform(const Tree::Expr& e);

    Tree::Expr
    operator()(const Tree::WhereExpr& where);

    Tree::Expr
    operator()(const Tree::LambdaExpr& e);

    Tree::Expr
    operator()(const Tree::PhiExpr& e);

    const std::vector<Parser::Equation>& 
    newVars() const
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
    System& m_system;

    std::vector<dimension_index> m_Lout;
    std::vector<dimension_index> m_Lin;

    std::vector<Parser::Equation> m_newVars;

    //we actually only need one scope stack
    std::vector<dimension_index> m_scope;

    std::vector<u32string> m_scopeNames;

    std::vector<dimension_index> m_namedAllScopeArgs;
    std::vector<dimension_index> m_namedAllScopeOdometers;
  };
}

#endif
