/* Removes call-by-name expressions
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
 * @file remove_cbn.hpp
 * Removes call by name according to step 2 of 
 * The Intension as First-class Value
 * The input tree must:
 * 1. Be rewritten with TreeRewriter
 * 2. Have names renamed so that they are unique,
 *
 * The output is a tree rewritten according to the transformation.
 */

#ifndef TL_FUNCTION_TRANSFORM_HPP_INCLUDED
#define TL_FUNCTION_TRANSFORM_HPP_INCLUDED

#include <tl/generic_walker.hpp>

#include <set>

namespace TransLucid
{
  class FunctionTransform : public GenericTreeWalker<FunctionTransform>
  {
    public:
    typedef Tree::Expr result_type;

    using GenericTreeWalker::operator();

    Tree::Expr
    transform(const Tree::Expr& e)
    {
      return apply_visitor(*this, e);
    }

    Tree::Expr
    operator()(const Tree::IdentExpr& e)
    {
      if (m_cbnscope.find(e.text) != m_cbnscope.end())
      {
        return Tree::EvalIntenExpr(e);
      }
      else
      {
        return e;
      }
    }

    Tree::Expr
    operator()(const Tree::PhiExpr& e)
    {
      m_cbnscope.insert(e.name);

      Tree::LambdaExpr lambda = Tree::LambdaExpr(e.name, e.rhs);
      lambda.argDim = e.argDim;;

      Tree::Expr le = lambda;

      Tree::Expr fun = apply_visitor(*this, le);

      m_cbnscope.erase(e.name);

      return fun;
    }

    Tree::Expr
    operator()(const Tree::PhiAppExpr& e)
    {
      return Tree::LambdaAppExpr
      (
        apply_visitor(*this, e.lhs),
        Tree::MakeIntenExpr(apply_visitor(*this, e.rhs))
      );
    }

    private:
    std::set<u32string> m_cbnscope;
  };
}

#endif
