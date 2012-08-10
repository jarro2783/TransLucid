/* Abstract syntax tree.
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

/**
 * @file ast.hpp
 * Everything related to abstract syntax trees.
 * Contains everything needed to build and traverse an abstract syntax tree.
 */

#ifndef TL_AST_FWD_HPP_INCLUDED
#define TL_AST_FWD_HPP_INCLUDED

#include <tl/types_basic.hpp>
#include <tl/gmpxx_fwd.hpp>

namespace TransLucid
{
  template <typename First, typename... Types>
  class Variant;

  template <typename T>
  class recursive_wrapper;

  namespace Tree
  {
    struct nil;
    struct LiteralExpr;
    struct DimensionExpr;
    struct IdentExpr;
    struct HashSymbol;
    struct ParenExpr;
    struct UnaryOpExpr;
    struct BinaryOpExpr;
    struct MakeIntenExpr;
    struct EvalIntenExpr;
    struct HashExpr;
    struct HostOpExpr;
    struct TupleExpr;
    struct IfExpr;
    struct AtExpr;
    struct LambdaExpr;
    struct PhiExpr;
    struct BaseAbstractionExpr;
    struct BangAppExpr;
    struct LambdaAppExpr;
    struct PhiAppExpr;
    struct WhereExpr;
    struct ConditionalBestfitExpr;

   /**
     * Abstract syntax tree node. A single expression node in the 
     * abstract syntax tree which is created by the parser.
     */
    typedef Variant
    <
      nil,
      bool,
      Special,     //replaces SpecialExpr
      mpz_class,          //replaces IntegerExpr
      char32_t,           //replaces UcharExpr
      u32string,          //replaces StringExpr
      LiteralExpr,
      DimensionExpr,
      IdentExpr,
      HashSymbol,
      HostOpExpr,
      recursive_wrapper<ParenExpr>,
      recursive_wrapper<UnaryOpExpr>,
      recursive_wrapper<BinaryOpExpr>,
      recursive_wrapper<MakeIntenExpr>,
      recursive_wrapper<EvalIntenExpr>,
      recursive_wrapper<IfExpr>,
      recursive_wrapper<HashExpr>,
      recursive_wrapper<TupleExpr>,
      recursive_wrapper<AtExpr>,
      recursive_wrapper<LambdaExpr>,
      recursive_wrapper<PhiExpr>,
      recursive_wrapper<BaseAbstractionExpr>,
      recursive_wrapper<BangAppExpr>,
      recursive_wrapper<LambdaAppExpr>,
      recursive_wrapper<PhiAppExpr>,
      recursive_wrapper<WhereExpr>,
      recursive_wrapper<ConditionalBestfitExpr>
    > Expr;

    struct BinaryOperator;
    struct UnaryOperator;
  }

  namespace Parser
  {
    typedef std::tuple<u32string, Tree::Expr, Tree::Expr, Tree::Expr> 
    Equation;
  }
}

#endif
