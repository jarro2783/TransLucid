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

#include <tl/tree_to_wstree.hpp>

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
  return Tree::AtExpr
  (
    Tree::IdentExpr(U"LITERAL"),
    Tree::TupleExpr
    (
      {
        {Tree::DimensionExpr(U"type"), e.type},
        {Tree::DimensionExpr(U"text"), e.text}
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
  //UNOP @ [arg0 : e.e, opname : e.op.op]
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
}

Tree::Expr TreeToWSTree::operator()(const Tree::BinaryOpExpr& e)
{
  //BINOP @ [arg0 : e.lhs, arg1 : e.rhs, opname : e.op.op]
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
}

Tree::Expr
TreeToWSTree::operator()(const Tree::BangOpExpr& e)
{
  std::vector<Tree::Expr> args;

  for (auto expr : e.args)
  {
    args.push_back(boost::apply_visitor(*this, expr));
  }

  return Tree::BangOpExpr(e.name, args);
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


}
