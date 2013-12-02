/* Translates AST::Expr to workshops.
   Copyright (C) 2009--2012 Jarryd Beck

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

/** @file workshop_builder.hpp
 * Expression compiler definition.
 * Compiles a tree to workshops.
 */

#ifndef EXPR_COMPILER_HPP_INCLUDED
#define EXPR_COMPILER_HPP_INCLUDED

#include <tl/ast.hpp>

namespace TransLucid
{

  class System;

  class WorkshopBuilder
  {
    public:
    //boost::apply_visitor requires this
    typedef WS* result_type;

    WorkshopBuilder(System* system);
    ~WorkshopBuilder();

    WS* build_workshops(const Tree::Expr&);
    //WS* compile_top_level(const Tree::Expr&);

    WS* operator()(const Tree::nil& n);
    WS* operator()(bool b);
    WS* operator()(Special s);
    WS* operator()(const mpz_class& i);
    WS* operator()(char32_t c);
    WS* operator()(const u32string& s);
    WS* operator()(const Constant& c);
    WS* operator()(const Tree::HashSymbol& e);
    WS* operator()(const Tree::LiteralExpr& e);
    WS* operator()(const Tree::DimensionExpr& e);
    WS* operator()(const Tree::IdentExpr& e);
    WS* operator()(const Tree::ParenExpr& e);
    WS* operator()(const Tree::UnaryOpExpr& e);
    WS* operator()(const Tree::BinaryOpExpr& e);
    WS* operator()(const Tree::MakeIntenExpr& e);
    WS* operator()(const Tree::EvalIntenExpr& e);
    WS* operator()(const Tree::IfExpr& e);
    WS* operator()(const Tree::HashExpr& e);
    WS* operator()(const Tree::HostOpExpr& e);
    WS* operator()(const Tree::RegionExpr& e);
    WS* operator()(const Tree::TupleExpr& e);
    WS* operator()(const Tree::AtExpr& e);
    WS* operator()(const Tree::LambdaExpr& e);
    WS* operator()(const Tree::PhiExpr& e);
    WS* operator()(const Tree::BaseAbstractionExpr& e);
    WS* operator()(const Tree::BangAppExpr& e);
    WS* operator()(const Tree::LambdaAppExpr& e);
    WS* operator()(const Tree::PhiAppExpr& e);
    WS* operator()(const Tree::WhereExpr& e);
    WS* operator()(const Tree::ConditionalBestfitExpr& e);

    private:
    //the system to compile with
    System* m_system;

    //the lambda scope
    std::vector<dimension_index> m_lambdaScope;

    //the phi scope
    std::vector<dimension_index> m_phiScope;

    //the where L_{out}s
    std::vector<dimension_index> m_whereOut;
  };

} //namespace TransLucid

#endif // EXPR_COMPILER_HPP_INCLUDED
