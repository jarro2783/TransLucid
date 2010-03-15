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

#if 0

#ifndef EVALUATOR_HPP_INCLUDED
#define EVALUATOR_HPP_INCLUDED

#include <tl/expr.hpp>
#include <tl/types.hpp>
#include <tl/dimtranslator.hpp>
#include <tl/builtin_types.hpp>

namespace TransLucid
{

  class Interpreter;

  class Evaluator : public AST::Visitor
  {
    public:

    Evaluator(Interpreter& i);

    ValueContext evaluate(AST::Expr* e);
    ValueContext evaluate(AST::Expr* e, const Tuple& c);

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
    AST::Data* visitOpExpr(AST::OpExpr*, AST::Data*);
    AST::Data* visitPairExpr(AST::PairExpr*, AST::Data*);
    AST::Data* visitRangeExpr(AST::RangeExpr*, AST::Data*);
    AST::Data* visitUnaryExpr(AST::UnaryExpr*, AST::Data*);

    private:
    Interpreter& m_interpreter;
    TypeRegistry& m_registry;
    DimensionTranslator& m_dims;

    TypedValue makeSpecial(Special::Value value);
    TypedValue makeBoolean(bool b);
    TypedValue makeDimension(size_t v);
    size_t lookupDim(const TypedValue& v);
  };

}

#endif // EVALUATOR_HPP_INCLUDED

#endif //if 0
