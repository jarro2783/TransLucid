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

    Tree::Expr 
    operator()(const Tree::HashExpr& e)
    {
      return Tree::HashExpr(
        apply_visitor(*reinterpret_cast<Derived*>(this), e.e));
    }

    Tree::Expr
    operator()(const Tree::BaseAbstractionExpr& e)
    {
      return e;
    }

    Tree::Expr
    operator()(const Tree::RegionExpr& e)
    {
      Tree::RegionExpr::Entries entries;

      for (const auto& entry : e.entries)
      {
        entries.push_back(std::make_tuple
        (
          apply_visitor(*reinterpret_cast<Derived*>(this), std::get<0>(entry)),
          std::get<1>(entry),
          apply_visitor(*reinterpret_cast<Derived*>(this), std::get<2>(entry))
        ));
      }

      return Tree::RegionExpr{entries};
    }

    Tree::Expr 
    operator()(const Tree::TupleExpr& e)
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
      std::vector<Tree::Expr> binds;

      for (auto& b : e.binds)
      {
        binds.push_back(apply_visitor(*reinterpret_cast<Derived*>(this), b));
      }

      return Tree::MakeIntenExpr
      {
        apply_visitor(*reinterpret_cast<Derived*>(this), e.expr), 
        binds,
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
      std::vector<Tree::Expr> binds;

      for (auto& b : e.binds)
      {
        binds.push_back(apply_visitor(*reinterpret_cast<Derived*>(this), b));
      }

      Tree::LambdaExpr le
      (
        e.name,
        binds,
        apply_visitor(*reinterpret_cast<Derived*>(this), e.rhs)
      );

      le.scope = e.scope;

      le.argDim = e.argDim;

      return le;
    }

    Tree::Expr operator()(const Tree::PhiExpr& e)
    {
      std::vector<Tree::Expr> binds;

      for (auto& b : e.binds)
      {
        binds.push_back(apply_visitor(*reinterpret_cast<Derived*>(this), b));
      }

      Tree::PhiExpr phi
      (
        e.name,
        binds,
        apply_visitor(*reinterpret_cast<Derived*>(this), e.rhs)
      );

      phi.argDim = e.argDim;
      phi.scope = e.scope;

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

      where.e = apply_visitor(*reinterpret_cast<Derived*>(this), e.e);

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

      //do the funs
      for (const auto& fun : e.funs)
      {
        where.funs.push_back
        (
          Parser::FnDecl
          {
            fun.name,
            fun.args,
            apply_visitor(*reinterpret_cast<Derived*>(this), fun.guard),
            apply_visitor(*reinterpret_cast<Derived*>(this), fun.boolean),
            apply_visitor(*reinterpret_cast<Derived*>(this), fun.expr)
          }
        );
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

      cond.name = e.name;

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

  //visit a tree and do nothing
  //this only makes sense if the derived class does something interesting
  //with some of the nodes as a side effect
  template <typename Self>
  class GenericTreeVisitor
  {
    public:

    template <typename T>
    void
    operator()(const T&)
    {
    }

    void
    operator()(const Tree::ParenExpr& e)
    {
      apply_visitor(*reinterpret_cast<Self*>(this), e.e);
    }

    void
    operator()(const Tree::MakeIntenExpr& e)
    {
      for (const auto& b : e.binds)
      {
        apply_visitor(*reinterpret_cast<Self*>(this), b);
      }

      apply_visitor(*reinterpret_cast<Self*>(this), e.expr);
    }

    void
    operator()(const Tree::EvalIntenExpr& e)
    {
      apply_visitor(*reinterpret_cast<Self*>(this), e.expr);
    }

    void
    operator()(const Tree::IfExpr& e)
    {
      apply_visitor(*reinterpret_cast<Self*>(this), e.condition);
      apply_visitor(*reinterpret_cast<Self*>(this), e.then);

      for (const auto& eifs : e.else_ifs)
      {
        apply_visitor(*reinterpret_cast<Self*>(this), eifs.first);
        apply_visitor(*reinterpret_cast<Self*>(this), eifs.second);
      }

      apply_visitor(*reinterpret_cast<Self*>(this), e.else_);
    }

    void
    operator()(const Tree::HashExpr& e)
    {
      apply_visitor(*reinterpret_cast<Self*>(this), e.e);
    }

    void
    operator()(const Tree::RegionExpr& e)
    {
      for (const auto& entry : e.entries)
      {
        apply_visitor(*reinterpret_cast<Self*>(this), std::get<0>(entry));
        apply_visitor(*reinterpret_cast<Self*>(this), std::get<2>(entry));
      }
    }

    void
    operator()(const Tree::TupleExpr& e)
    {
      for (const auto& entry : e.pairs)
      {
        apply_visitor(*reinterpret_cast<Self*>(this), entry.first);
        apply_visitor(*reinterpret_cast<Self*>(this), entry.second);
      }
    }

    void
    operator()(const Tree::AtExpr& e)
    {
      apply_visitor(*reinterpret_cast<Self*>(this), e.lhs);
      apply_visitor(*reinterpret_cast<Self*>(this), e.rhs);
    }

    void
    operator()(const Tree::LambdaExpr& e)
    {
      for (const auto& b : e.binds)
      {
        apply_visitor(*reinterpret_cast<Self*>(this), b);
      }

      apply_visitor(*reinterpret_cast<Self*>(this), e.rhs);
    }

    void
    operator()(const Tree::BaseAbstractionExpr& e)
    {
      for (const auto& b : e.binds)
      {
        apply_visitor(*reinterpret_cast<Self*>(this), b);
      }

      apply_visitor(*reinterpret_cast<Self*>(this), e.body);
    }

    void
    operator()(const Tree::BangAppExpr& e)
    {
      for (const auto& a : e.args)
      {
        apply_visitor(*reinterpret_cast<Self*>(this), a);
      }

      apply_visitor(*reinterpret_cast<Self*>(this), e.name);
    }

    void
    operator()(const Tree::LambdaAppExpr& e)
    {
      apply_visitor(*reinterpret_cast<Self*>(this), e.lhs);
      apply_visitor(*reinterpret_cast<Self*>(this), e.rhs);
    }

    void
    operator()(const Tree::ConditionalBestfitExpr& e)
    {
      for (const auto& d : e.declarations)
      {
        apply_visitor(*reinterpret_cast<Self*>(this), std::get<1>(d));
        apply_visitor(*reinterpret_cast<Self*>(this), std::get<2>(d));
        apply_visitor(*reinterpret_cast<Self*>(this), std::get<3>(d));
      }
    }
  };
}

#endif
