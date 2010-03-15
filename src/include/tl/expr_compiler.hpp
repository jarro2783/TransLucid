/* TODO: Give a descriptor.
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

#include <tl/expr.hpp>
#include <tl/ast.hpp>

namespace TransLucid
{

  class ExprCompiler : public AST::Visitor
  {
    public:

    ExprCompiler(HD* i);
    ~ExprCompiler();

    HD* compile(AST::Expr*);
    HD* compile(const Tree::Expr&);

    AST::Data* visitAtExpr(AST::AtExpr*, AST::Data*);
    AST::Data* visitBinaryOpExpr(AST::BinaryOpExpr*, AST::Data*);
    AST::Data* visitBooleanExpr(AST::BooleanExpr*, AST::Data*);
    AST::Data* visitBuildTupleExpr(AST::BuildTupleExpr*, AST::Data*);
    AST::Data* visitConstantExpr(AST::ConstantExpr*, AST::Data*);
    AST::Data* visitConvertExpr(AST::SpecialOpsExpr*, AST::Data*);
    AST::Data* visitDimensionExpr(AST::DimensionExpr*, AST::Data*);
    AST::Data* visitHashExpr(AST::HashExpr*, AST::Data*);
    AST::Data* visitIdentExpr(AST::IdentExpr*, AST::Data*);
    AST::Data* visitIfExpr(AST::IfExpr*, AST::Data*);
    AST::Data* visitIntegerExpr(AST::IntegerExpr*, AST::Data*);
    AST::Data* visitIsSpecialExpr(AST::SpecialOpsExpr*, AST::Data*);
    AST::Data* visitIsTypeExpr(AST::SpecialOpsExpr*, AST::Data*);
    AST::Data* visitPairExpr(AST::PairExpr*, AST::Data*);
    AST::Data* visitOpExpr(AST::OpExpr*, AST::Data*);
    AST::Data* visitRangeExpr(AST::RangeExpr*, AST::Data*);
    AST::Data* visitSpecialExpr(AST::SpecialExpr*, AST::Data*);
    AST::Data* visitStringExpr(AST::StringExpr*, AST::Data*);
    AST::Data* visitUcharExpr(AST::UcharExpr*, AST::Data*);
    AST::Data* visitUnaryExpr(AST::UnaryExpr*, AST::Data*);

    //the new boost variant visitors
    typedef HD* result_type;

    HD* operator()(const Tree::nil& n);
    HD* operator()(const Tree::AtExpr& e);
    HD* operator()(bool b);
    HD* operator()(const Tree::BinaryOpExpr& e);
    HD* operator()(const Tree::BuildTupleExpr& e);
    HD* operator()(const Tree::ConstantExpr& e);
    HD* operator()(char32_t c);
    HD* operator()(const Tree::DimensionExpr& e);
    HD* operator()(const Tree::HashExpr& e);
    HD* operator()(const Tree::IdentExpr& e);
    HD* operator()(const Tree::IfExpr& e);
    HD* operator()(const mpz_class& i);
    HD* operator()(Special::Value s);
    HD* operator()(const u32string& s);
    HD* operator()(const Tree::UnaryOpExpr& e);

    #if 0
    boost::recursive_wrapper<AtExpr>,
    boost::recursive_wrapper<BinaryOpExpr>,
    boost::recursive_wrapper<BuildTupleExpr>,
    ConstantExpr,
    DimensionExpr,
    boost::recursive_wrapper<HashExpr>,
    IdentExpr,
    boost::recursive_wrapper<IfExpr>,
    boost::recursive_wrapper<UnaryExpr>,
    bool,
    char32_t, //replaces UcharExpr
    mpz_class, //replaces IntegerExpr
    Special::Value, //replaces SpecialExpr
    u32string //replaces StringExpr
    #endif

    private:
    HD* m_i;
  };

} //namespace TransLucid

#endif // EXPR_COMPILER_HPP_INCLUDED
