/* Replaces free variables with #hidden
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

#include <vector>
#include <unordered_set>
#include <utility>

#include <tl/ast_fwd.hpp>
#include <tl/types_basic.hpp>

#include "tl/generic_walker.hpp"

namespace TransLucid
{
  class System;

  class FreeVariableReplacer : private GenericTreeWalker<FreeVariableReplacer>
  {
    public:
    using GenericTreeWalker<FreeVariableReplacer>::operator();

    typedef std::vector<std::pair<u32string, dimension_index>> Replaced;

    typedef Tree::Expr result_type;

    FreeVariableReplacer(System& system)
    : m_system(system) {}

    const Replaced&
    getReplaced();

    Tree::Expr
    replaceFree(const Tree::Expr& expr);

    template <typename T>
    Tree::Expr
    operator()(const T& e)
    {
      return e;
    }

    Tree::Expr operator()(const Tree::IdentExpr& e);
    Tree::Expr operator()(const Tree::LambdaExpr& e);
    Tree::Expr operator()(const Tree::PhiExpr& e);
    Tree::Expr operator()(const Tree::WhereExpr& e);

    private:
    System& m_system;
    Replaced m_replaced;
    std::unordered_set<u32string> m_bound;
  };
}
