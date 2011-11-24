/* Tree walk when not much changes.
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

    Tree::Expr operator()(const Tree::AtExpr& e)
    {
      return Tree::AtExpr
      (
        apply_visitor(*reinterpret_cast<Derived*>(this), e.lhs),
        apply_visitor(*reinterpret_cast<Derived*>(this), e.rhs)
      );
    }

    Tree::Expr operator()(const Tree::LambdaExpr& e);

    Tree::Expr operator()(const Tree::PhiExpr& e);

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

    Tree::Expr operator()(const Tree::WhereExpr& e);

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
