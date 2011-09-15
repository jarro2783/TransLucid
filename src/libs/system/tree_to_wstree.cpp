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

/** @file tree_to_wstree.cpp
 * Rewrites an expression tree to remove operators and literals.
 */

#include <tl/tree_to_wstree.hpp>
#include <tl/internal_strings.hpp>

namespace TransLucid
{

Tree::Expr
toWSTree(const Tree::Expr& expr)
{
  TreeToWSTree t;
  return boost::apply_visitor(t, expr);
}

Tree::Expr TreeToWSTree::operator()(const Tree::nil& n)
{
  return n;
}

Tree::Expr TreeToWSTree::operator()(bool b)
{
  return b;
}

Tree::Expr TreeToWSTree::operator()(Special s)
{
  return s;
}

Tree::Expr TreeToWSTree::operator()(const mpz_class& i)
{
  return i;
}

Tree::Expr TreeToWSTree::operator()(char32_t c)
{
  return c;
}

Tree::Expr TreeToWSTree::operator()(const u32string& s)
{
  return s;
}

Tree::Expr TreeToWSTree::operator()(const Tree::LiteralExpr& e)
{
  //LITERAL @ [type : e.type, text : e.text]
  #if 0
  std::cerr << "Translating LiteralExpr " << e.type << "\"" << e.text
            << "\" to" << std::endl
            << "LITERAL @ [typename <- \"" << e.type << "\", text <- \""
            << e.text << "\"]" << std::endl;
  #endif
  return Tree::AtExpr
  (
    Tree::IdentExpr(LITERAL_IDENT),
    Tree::TupleExpr
    (
      {
        {Tree::DimensionExpr(type_name_dim), e.type},
        {Tree::DimensionExpr(text_dim), e.text}
      }
    )
  );
}

Tree::Expr TreeToWSTree::operator()(const Tree::DimensionExpr& e)
{
  return e;
}

Tree::Expr TreeToWSTree::operator()(const Tree::IdentExpr& e)
{
  return e;
}

Tree::Expr TreeToWSTree::operator()(const Tree::ParenExpr& e)
{
  return boost::apply_visitor(*this, e.e);
}

Tree::Expr TreeToWSTree::operator()(const Tree::UnaryOpExpr& e)
{
  //(FN1 ! (#arg0)) @ [fnname <- e.op.op, arg0 <- T(e.e)]

  //for now just get this working, we can optimise by only calculating the
  //expr once, but how do I do that...
  //
  Tree::DimensionExpr arg0(U"arg0");

  return 
  Tree::AtExpr
  (
    Tree::BangOpExpr
    (
      Tree::IdentExpr(FN1_IDENT),
      {Tree::HashExpr(arg0)}
    ),
    Tree::TupleExpr
    (
      {
        {Tree::DimensionExpr(fnname_dim), e.op.op},
        {arg0, boost::apply_visitor(*this, e.e)}
      }
    )
  );

  #if 0
  //UNOP @ [arg0 <- e.e, opname <- e.op.op]
  return Tree::AtExpr
  (
    Tree::IdentExpr(U"UNOP"),
    Tree::TupleExpr
    (
      {
        {Tree::DimensionExpr(U"arg0"), boost::apply_visitor(*this, e.e)},
        {Tree::DimensionExpr(U"opname"), e.op.op}
      }
    )
  );
  #endif
}

Tree::Expr TreeToWSTree::operator()(const Tree::BinaryOpExpr& e)
{
  //(FN2 ! (#arg0, #arg1))
  //  @ [fnname <- e.op.op, arg0 <- T(e.lhs), arg1 <- T(e.rhs)]

  Tree::Expr elhs = boost::apply_visitor(*this, e.lhs);
  Tree::Expr erhs = boost::apply_visitor(*this, e.rhs);

  Tree::DimensionExpr arg0(U"arg0");
  Tree::DimensionExpr arg1(U"arg1");

  //optimise as above
  return 
  
  Tree::AtExpr
  (
    Tree::BangOpExpr
    (
      Tree::IdentExpr(FN2_IDENT),
      {
        Tree::HashExpr(arg0),
        Tree::HashExpr(arg1)
      }
    ),
    Tree::TupleExpr
    (
      {
        {Tree::DimensionExpr(U"arg0"), elhs},
        {Tree::DimensionExpr(U"arg1"), erhs},
        {Tree::DimensionExpr(fnname_dim), e.op.op}
      }
    )
  );


  #if 0
  //BINOP @ [arg0 <- e.lhs, arg1 <- e.rhs, opname <- e.op.op]
  return Tree::AtExpr
  (
    Tree::IdentExpr(U"BINOP"),
    Tree::TupleExpr
    (
      {
        {Tree::DimensionExpr(U"arg0"), boost::apply_visitor(*this, e.lhs)},
        {Tree::DimensionExpr(U"arg1"), boost::apply_visitor(*this, e.rhs)},
        {Tree::DimensionExpr(U"opname"), e.op.op}
      }
    )
  );
  #endif
}

Tree::Expr
TreeToWSTree::operator()(const Tree::BangOpExpr& e)
{
  Tree::Expr name = boost::apply_visitor(*this, e.name);
  std::vector<Tree::Expr> args;

  for (auto expr : e.args)
  {
    args.push_back(boost::apply_visitor(*this, expr));
  }

  return Tree::BangOpExpr(name, args);
}

Tree::Expr TreeToWSTree::operator()(const Tree::IfExpr& e)
{
  //do the elseifs
  std::vector<std::pair<Tree::Expr, Tree::Expr>> else_ifs;
  for (auto p : e.else_ifs)
  {
    else_ifs.push_back(std::make_pair(
      boost::apply_visitor(*this, p.first),
      boost::apply_visitor(*this, p.second)
    ));
  }

  return Tree::IfExpr(
    boost::apply_visitor(*this, e.condition),
    boost::apply_visitor(*this, e.then),
    else_ifs,
    boost::apply_visitor(*this, e.else_)
  );
}

Tree::Expr TreeToWSTree::operator()(const Tree::HashExpr& e)
{
  return Tree::HashExpr(boost::apply_visitor(*this, e.e));
}

Tree::Expr TreeToWSTree::operator()(const Tree::TupleExpr& e)
{
  std::vector<std::pair<Tree::Expr, Tree::Expr>> tuple;
  for (auto p : e.pairs)
  {
    tuple.push_back(std::make_pair(
      boost::apply_visitor(*this, p.first),
      boost::apply_visitor(*this, p.second)
    ));
  }

  return Tree::TupleExpr(tuple);
}

Tree::Expr TreeToWSTree::operator()(const Tree::AtExpr& e)
{
  return Tree::AtExpr(
    boost::apply_visitor(*this, e.lhs),
    boost::apply_visitor(*this, e.rhs)
  );
}

Tree::Expr TreeToWSTree::operator()(const Tree::PhiExpr& e)
{
  return e;
}

Tree::Expr TreeToWSTree::operator()(const Tree::LambdaExpr& e)
{
  return Tree::LambdaExpr(e.name, boost::apply_visitor(*this, e.rhs));
}

Tree::Expr TreeToWSTree::operator()(const Tree::NameAppExpr& e)
{
  return Tree::NameAppExpr(
    boost::apply_visitor(*this, e.lhs),
    boost::apply_visitor(*this, e.rhs)
  );
}

Tree::Expr TreeToWSTree::operator()(const Tree::ValueAppExpr& e)
{
  return Tree::ValueAppExpr(
    boost::apply_visitor(*this, e.lhs),
    boost::apply_visitor(*this, e.rhs)
  );
}

Tree::Expr TreeToWSTree::operator()(const Tree::WhereExpr& e)
{
}

}
