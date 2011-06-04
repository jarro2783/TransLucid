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
}

Tree::Expr TreeToWSTree::operator()(bool b)
{
}

Tree::Expr TreeToWSTree::operator()(Special::Value s)
{
}

Tree::Expr TreeToWSTree::operator()(const mpz_class& i)
{
}

Tree::Expr TreeToWSTree::operator()(char32_t c)
{
}

Tree::Expr TreeToWSTree::operator()(const u32string& s)
{
}

Tree::Expr TreeToWSTree::operator()(const Tree::LiteralExpr& e)
{
}

Tree::Expr TreeToWSTree::operator()(const Tree::DimensionExpr& e)
{
}

Tree::Expr TreeToWSTree::operator()(const Tree::IdentExpr& e)
{
}

Tree::Expr TreeToWSTree::operator()(const Tree::ParenExpr& e)
{
}

Tree::Expr TreeToWSTree::operator()(const Tree::UnaryOpExpr& e)
{
}

Tree::Expr TreeToWSTree::operator()(const Tree::BinaryOpExpr& e)
{
}

Tree::Expr TreeToWSTree::operator()(const Tree::IfExpr& e)
{
}

Tree::Expr TreeToWSTree::operator()(const Tree::HashExpr& e)
{
}

Tree::Expr TreeToWSTree::operator()(const Tree::TupleExpr& e)
{
}

Tree::Expr TreeToWSTree::operator()(const Tree::AtExpr& e)
{
}

Tree::Expr TreeToWSTree::operator()(const Tree::PhiExpr& e)
{
}

Tree::Expr TreeToWSTree::operator()(const Tree::LambdaExpr& e)
{
}

Tree::Expr TreeToWSTree::operator()(const Tree::NameAppExpr& e)
{
}

Tree::Expr TreeToWSTree::operator()(const Tree::ValueAppExpr& e)
{
}


}
