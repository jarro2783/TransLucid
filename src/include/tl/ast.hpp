/* Abstract syntax tree.
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

#ifndef AST_HPP_INCLUDED
#define AST_HPP_INCLUDED

#include <boost/variant.hpp>
#include <tl/types.hpp>
#include <gmpxx.h>
#include <tl/builtin_types.hpp>
#include <boost/fusion/include/vector.hpp>
#include <boost/fusion/container.hpp>

namespace TransLucid
{
  namespace Tree
  {
    enum UnaryType
    {
      UNARY_PREFIX,
      UNARY_POSTFIX
    };

    struct UnaryOperator
    {
      UnaryOperator() = default;

      UnaryOperator
      (
        const u32string& op,
        const u32string& symbol,
        UnaryType type
      )
      : op(op), symbol(symbol), type(type)
      {}

      u32string op;
      u32string symbol;
      UnaryType type;

      bool
      operator==(const UnaryOperator& rhs) const
      {
        return op == rhs.op && symbol == rhs.symbol && type == rhs.type;
      }
    };

    enum InfixAssoc
    {
      ASSOC_LEFT,
      ASSOC_RIGHT,
      ASSOC_NON,
      ASSOC_VARIABLE,
      ASSOC_COMPARISON
    };

    struct BinaryOperator
    {
      BinaryOperator() = default;

      BinaryOperator
      (
        InfixAssoc assoc,
        const u32string& op,
        const u32string& symbol,
        const mpz_class& precedence
      )
      : op(op), symbol(symbol), assoc(assoc), precedence(precedence)
      {}

      bool
      operator==(const BinaryOperator& rhs) const
      {
        return op == rhs.op && symbol == rhs.symbol &&
        assoc == rhs.assoc && precedence == rhs.precedence;
      }

      bool
      operator!=(const BinaryOperator& rhs) const
      {
        return !(*this == rhs);
      }

      std::u32string op;
      std::u32string symbol;
      InfixAssoc assoc;
      mpz_class precedence;
    };

    struct nil
    {
    };

    struct ConstantExpr
    {
      ConstantExpr() = default;

      ConstantExpr(const u32string& t, const u32string& v)
      : type(t), text(v)
      {}

      u32string type;
      u32string text;
    };

    struct DimensionExpr
    {
      DimensionExpr() = default;

      DimensionExpr(const u32string& text)
      : text(text)
      {}

      u32string text;
    };

    struct IdentExpr
    {
      IdentExpr() = default;

      IdentExpr(const u32string& t)
      : text(t)
      {}

      u32string text;
    };

    class UnaryOpExpr;
    class BinaryOpExpr;
    class HashExpr;
    class TupleExpr;
    class IfExpr;
    class AtExpr;

    // Not defined in ast.hpp
    class OpExpr;
    class RangeExpr;
    //TODO: rename ConstantExpr to TypedValueExpr
    typedef boost::variant
    <
      nil,
      bool,
      Special::Value,     //replaces SpecialExpr
      mpz_class,          //replaces IntegerExpr
      char32_t,           //replaces UcharExpr
      u32string,          //replaces StringExpr
      ConstantExpr,
      DimensionExpr,
      IdentExpr,
      boost::recursive_wrapper<UnaryOpExpr>,
      boost::recursive_wrapper<BinaryOpExpr>,
      boost::recursive_wrapper<IfExpr>,
      boost::recursive_wrapper<HashExpr>,
      boost::recursive_wrapper<TupleExpr>,
      boost::recursive_wrapper<AtExpr>
    > Expr;

    struct UnaryOpExpr
    {
      UnaryOpExpr() = default;

      UnaryOpExpr(const UnaryOperator& o, const Expr& e)
      : op(o), e(e)
      {}

      UnaryOperator op;
      Expr e;
    };

    struct BinaryOpExpr
    {
      BinaryOpExpr() = default;

      BinaryOpExpr
      (
        BinaryOperator o,
        const Expr& l,
        const Expr& r
      )
      : op(o), lhs(l), rhs(r)
      {}

      BinaryOperator op;
      Expr lhs;
      Expr rhs;

      void
      add_right(const BinaryOperator& op, Expr& rhs);

      void
      add_leaf(Expr& e);
    };

    Expr
    insert_binary_operator
    (
      const BinaryOperator& info,
      Expr& lhs,
      Expr& rhs
    );

    struct IfExpr
    {
      IfExpr() = default;

      template <typename List>
      IfExpr
      (
        const Expr& c,
        const Expr& t,
        const List& eif,
        const Expr& e
      )
      : condition(c),
        then(t),
        else_(e)
      {
        using boost::fusion::at_c;

        BOOST_FOREACH(auto& v, eif)
        {
          else_ifs.push_back(std::make_pair(at_c<0>(v), at_c<1>(v)));
        }
      }

      Expr condition;
      Expr then;
      std::vector<std::pair<Expr, Expr>> else_ifs;
      Expr else_;
    };

    struct HashExpr
    {
      HashExpr() = default;

      HashExpr(const Expr& e)
      : e(e)
      {}

      Expr e;
    };

    struct TupleExpr
    {
      typedef
      std::vector<boost::fusion::vector<Expr, Expr>>
      TuplePairs;

      TuplePairs pairs;

      TupleExpr() = default;

      TupleExpr(const TuplePairs& p)
      : pairs(p)
      {}
    };

    #warning implement absolute @ 
    struct AtExpr
    {
      AtExpr() = default;

      AtExpr(const Expr& lhs, const Expr& rhs)
      : lhs(lhs), rhs(rhs), relative(true)
      {}

      Expr lhs;
      Expr rhs;
      bool relative;
    };

    #define PRINT_NODE(n) \
    inline \
    std::ostream& operator<<(std::ostream& os, const n &) \
    { \
       os << #n ; \
       return os; \
    } \

    PRINT_NODE(nil)
    PRINT_NODE(ConstantExpr)
    PRINT_NODE(DimensionExpr)
    PRINT_NODE(IdentExpr)
    PRINT_NODE(UnaryOpExpr)
    PRINT_NODE(BinaryOpExpr)
    PRINT_NODE(IfExpr)
    PRINT_NODE(HashExpr)
    PRINT_NODE(TupleExpr)
    PRINT_NODE(AtExpr)

  }
}

#endif // AST_HPP_INCLUDED
