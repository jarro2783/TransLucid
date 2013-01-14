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
#include <tl/semantics.hpp>

#include <set>
#include <stack>
#include <unordered_map>

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

    void
    restoreScope(const ScopePtr& scope);

    Tree::Expr
    transform(const Tree::Expr& e);
    
    Tree::Expr 
    operator()(const Tree::IdentExpr& e);

    Tree::Expr 
    operator()(const Tree::ParenExpr& e);

    Tree::Expr 
    operator()(const Tree::LiteralExpr& e);

    Tree::Expr 
    operator()(const Tree::UnaryOpExpr& e);

    Tree::Expr 
    operator()(const Tree::BinaryOpExpr& e);

    Tree::Expr 
    operator()(const Tree::BangAppExpr& e);

    Tree::Expr
    operator()(const Tree::WhereExpr& where);

    Tree::Expr
    operator()(const Tree::MakeIntenExpr& e);

    Tree::Expr
    operator()(const Tree::BaseAbstractionExpr& e);

    Tree::Expr
    operator()(const Tree::LambdaExpr& e);

    Tree::Expr
    operator()(const Tree::PhiExpr& e);

    Tree::Expr
    operator()(const Tree::PhiAppExpr& e);

    private:
    typedef std::unordered_map<u32string, dimension_index> ParameterReplaced;

    ScopePtr
    makeScope() const;

    //open a new scope, possibly shadowing another
    u32string
    pushScope(const u32string& id);

    void
    popScope(const u32string& id);

    //get the current rename for this identifier
    const u32string& 
    getRenamed(const u32string& name);

    System& m_system;

    //the scope of dimensions to save
    std::vector<dimension_index> m_scope;

    //the names of what is in scope
    std::vector<u32string> m_scopeNames;

    //function parameters to replace
    ParameterReplaced m_fnScope;

    //which ones are call by name
    std::set<u32string> m_cbnscope;

    //which ones are local dimenions
    std::set<u32string> m_dimscope;

    //the renames
    //a hashmap would be better, but we need to hold on to iterators
    //while we're changing it and they can't be invalidated
    typedef std::map<u32string, std::stack<u32string>> RenameRules;

    RenameRules m_rename;

    std::vector<std::pair<ScopePtr, Parser::Line>> m_newVars;

    bool
    caching() const;

    public:
    const decltype(m_newVars)&
    newVars() const
    {
      return m_newVars;
    }
  };
}

#endif
