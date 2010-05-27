/* Translates AST::Expr to hyperdatons.
   Copyright (C) 2009, 2010 Jarryd Beck and John Plaice

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

#ifndef EXPR_COMPILER_HPP_INCLUDED
#define EXPR_COMPILER_HPP_INCLUDED

#include <tl/ast.hpp>

namespace TransLucid
{

  class ExprCompiler
  {
    public:
    //boost::apply_visitor requires this
    typedef HD* result_type;

    ExprCompiler(HD* i);
    ~ExprCompiler();

    HD* compile(const Tree::Expr&);

    HD* operator()(const Tree::nil& n);
    HD* operator()(bool b);
    HD* operator()(Special::Value s);
    HD* operator()(const mpz_class& i);
    HD* operator()(char32_t c);
    HD* operator()(const u32string& s);
    HD* operator()(const Tree::ConstantExpr& e);
    HD* operator()(const Tree::DimensionExpr& e);
    HD* operator()(const Tree::IdentExpr& e);
    HD* operator()(const Tree::UnaryOpExpr& e);
    HD* operator()(const Tree::BinaryOpExpr& e);
    HD* operator()(const Tree::IfExpr& e);
    HD* operator()(const Tree::HashExpr& e);
    HD* operator()(const Tree::TupleExpr& e);
    HD* operator()(const Tree::AtExpr& e);

    private:
    //the system to compile with
    HD* m_system;
  };

} //namespace TransLucid

#endif // EXPR_COMPILER_HPP_INCLUDED
