/* Abstract syntax tree.
   Copyright (C) 2009, 2010, 2011 Jarryd Beck and John Plaice

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
    class nil;
    class LiteralExpr;
    class DimensionExpr;
    class IdentExpr;
    class HashSymbol;
    class ParenExpr;
    class UnaryOpExpr;
    class BinaryOpExpr;
    class HashExpr;
    class TupleExpr;
    class IfExpr;
    class AtExpr;
    class BangExpr;
    class LambdaExpr;
    class PhiExpr;
    class BangAppExpr;
    class LambdaAppExpr;
    class PhiAppExpr;
    class WhereExpr;

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
      recursive_wrapper<ParenExpr>,
      recursive_wrapper<UnaryOpExpr>,
      recursive_wrapper<BinaryOpExpr>,
      recursive_wrapper<IfExpr>,
      recursive_wrapper<HashExpr>,
      recursive_wrapper<TupleExpr>,
      recursive_wrapper<AtExpr>,
      recursive_wrapper<BangExpr>,
      recursive_wrapper<LambdaExpr>,
      recursive_wrapper<PhiExpr>,
      recursive_wrapper<BangAppExpr>,
      recursive_wrapper<LambdaAppExpr>,
      recursive_wrapper<PhiAppExpr>,
      recursive_wrapper<WhereExpr>
    > Expr;

    class BinaryOperator;
    class UnaryOperator;
  }

  namespace Parser
  {
    typedef std::tuple<u32string, Tree::Expr, Tree::Expr, Tree::Expr> 
    Equation;
  }
}

#endif
