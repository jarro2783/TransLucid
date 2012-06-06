/* Tree walk when not much changes.
   Copyright (C) 2011, 2012 Jarryd Beck

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

#ifndef TL_GENERIC_WALKER_HPP_INCLUDED
#define TL_GENERIC_WALKER_HPP_INCLUDED

#include <tl/ast.hpp>

namespace TransLucid
{
  //transforms the tree by doing nothing
  //you want to use private inheritance to get all this functionality when
  //most of the expression nodes in your tree walking don't change anything
  template <class Derived>
  class GenericTreeWalker
  {
    public:
    template <typename T>
    Tree::Expr
    operator()(const T& e)
    {
      return e;
    }

    Tree::Expr operator()(const Tree::ParenExpr& e)
    {
      return Tree::ParenExpr(
        apply_visitor(*reinterpret_cast<Derived*>(this), e.e)
      );
    }

    Tree::Expr operator()(const Tree::UnaryOpExpr& e)
    {
      return Tree::UnaryOpExpr(e.op, 
        apply_visitor(*reinterpret_cast<Derived*>(this), e.e));
    }
    Tree::Expr operator()(const Tree::BinaryOpExpr& e)
    {
      return Tree::BinaryOpExpr
      (
        e.op,
        apply_visitor(*reinterpret_cast<Derived*>(this), e.lhs),
        apply_visitor(*reinterpret_cast<Derived*>(this), e.rhs)
      );
    }

    Tree::Expr operator()(const Tree::IfExpr& e)
    {
      return Tree::IfExpr
      (
        apply_visitor(*reinterpret_cast<Derived*>(this), e.condition),
        apply_visitor(*reinterpret_cast<Derived*>(this), e.then),
        visit_list(e.else_ifs),
        apply_visitor(*reinterpret_cast<Derived*>(this), e.else_)
      );
    }

    Tree::Expr operator()(const Tree::HashExpr& e)
    {
      return Tree::HashExpr(
        apply_visitor(*reinterpret_cast<Derived*>(this), e.e));
    }

    Tree::Expr
    operator()(const Tree::BaseAbstractionExpr& e)
    {
      return e;
    }

    Tree::Expr operator()(const Tree::TupleExpr& e)
    {
      std::vector<std::pair<Tree::Expr, Tree::Expr>> visited;

      for (auto iter = e.pairs.begin(); iter != e.pairs.end(); ++iter)
      {
        visited.push_back(std::make_pair
        (
          apply_visitor(*reinterpret_cast<Derived*>(this), iter->first),
          apply_visitor(*reinterpret_cast<Derived*>(this), iter->second)
        ));
      }

      return Tree::TupleExpr{visited};
    }

    Tree::Expr
    operator()(const Tree::MakeIntenExpr& e)
    {
      return Tree::MakeIntenExpr
      {
        apply_visitor(*reinterpret_cast<Derived*>(this), e.expr), 
        e.scope
      };
    }

    Tree::Expr
    operator()(const Tree::EvalIntenExpr& e)
    {
      return Tree::EvalIntenExpr
      {
        apply_visitor(*reinterpret_cast<Derived*>(this), e.expr), 
      };
    }

    Tree::Expr operator()(const Tree::AtExpr& e)
    {
      return Tree::AtExpr
      (
        apply_visitor(*reinterpret_cast<Derived*>(this), e.lhs),
        apply_visitor(*reinterpret_cast<Derived*>(this), e.rhs)
      );
    }

    Tree::Expr operator()(const Tree::LambdaExpr& e)
    {
      Tree::LambdaExpr le
      (
        e.name,
        apply_visitor(*reinterpret_cast<Derived*>(this), e.rhs)
      );

      le.argDim = e.argDim;
      le.scope = e.scope;
      le.free = e.free;

      return le;
    }

    Tree::Expr operator()(const Tree::PhiExpr& e)
    {
      Tree::PhiExpr phi
      (
        e.name,
        apply_visitor(*reinterpret_cast<Derived*>(this), e.rhs)
      );

      phi.argDim = e.argDim;
      phi.scope = e.scope;
      phi.free = e.free;

      return phi;
    }

    Tree::Expr operator()(const Tree::BangAppExpr& e)
    {
      std::vector<Tree::Expr> args;

      for (const auto& a : e.args)
      {
        args.push_back(apply_visitor(*reinterpret_cast<Derived*>(this), a));
      }

      return Tree::BangAppExpr
      (
        apply_visitor(*reinterpret_cast<Derived*>(this), e.name),
        std::move(args)
      );
    }

    Tree::Expr operator()(const Tree::LambdaAppExpr& e)
    {
      return Tree::LambdaAppExpr
      (
        apply_visitor(*reinterpret_cast<Derived*>(this), e.lhs),
        apply_visitor(*reinterpret_cast<Derived*>(this), e.rhs)
      );
    }

    Tree::Expr operator()(const Tree::PhiAppExpr& e)
    {
      return Tree::PhiAppExpr
      (
        apply_visitor(*reinterpret_cast<Derived*>(this), e.lhs),
        apply_visitor(*reinterpret_cast<Derived*>(this), e.rhs),
        e.Lall
      );
    }

    Tree::Expr operator()(const Tree::WhereExpr& e)
    {
      Tree::WhereExpr where;

      where.e = e.e;

      //do the dims
      for (const auto& dim : e.dims)
      {
        where.dims.push_back(std::make_pair(dim.first,
          apply_visitor(*reinterpret_cast<Derived*>(this), dim.second)));
      }

      //do the vars
      for (const auto& var : e.vars)
      {
        where.vars.push_back(std::make_tuple(
          std::get<0>(var),
          apply_visitor(*reinterpret_cast<Derived*>(this), std::get<1>(var)),
          apply_visitor(*reinterpret_cast<Derived*>(this), std::get<2>(var)),
          apply_visitor(*reinterpret_cast<Derived*>(this), std::get<3>(var))
        ));
      }

      return where;
    }

    Tree::Expr
    operator()(const Tree::ConditionalBestfitExpr& e)
    {
      Tree::ConditionalBestfitExpr cond;

      for (auto eqn : e.declarations)
      {
        cond.declarations.push_back
        (
          std::make_tuple
          (
            std::get<0>(eqn),
            apply_visitor(*reinterpret_cast<Derived*>(this), std::get<1>(eqn)),
            apply_visitor(*reinterpret_cast<Derived*>(this), std::get<2>(eqn)),
            apply_visitor(*reinterpret_cast<Derived*>(this), std::get<3>(eqn))
          )
        );
      }

      return cond;
    }

    std::vector<std::pair<Tree::Expr, Tree::Expr>>
    visit_list
    (
      const std::vector<std::pair<Tree::Expr, Tree::Expr>>& list
    )
    {
      std::vector<std::pair<Tree::Expr, Tree::Expr>> visited;

      for (auto iter = list.begin(); iter != list.end(); ++iter)
      {
        visited.push_back(std::make_pair
          (
            apply_visitor(*reinterpret_cast<Derived*>(this), iter->first),
            apply_visitor(*reinterpret_cast<Derived*>(this), iter->second)
          )
        );
      }

      return visited;
    }

  };
}

#endif
