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
  /** 
   * Abstract syntax tree. Contains all of the classes, types and functions
   * related to the abstract syntax tree.
   */
  namespace Tree
  {
    /** 
     * Type of unary operator. Symbols for the two types of unary operators.
     */
    enum UnaryType
    {
      /**
       * prefix operator.
       */
      UNARY_PREFIX,
      /**
       * postfix operator
       */
      UNARY_POSTFIX
    };

    /**
     * A parsed unary operator. Stores the symbol and the operator which
     * it maps to.
     */
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

      /**
       * The full name of the operation.
       */
      u32string op;

      /**
       * The symbol that was parsed to create this operation.
       */
      u32string symbol;

      /**
       * The type of operator.
       */
      UnaryType type;

      bool
      operator==(const UnaryOperator& rhs) const
      {
        return op == rhs.op && symbol == rhs.symbol && type == rhs.type;
      }
    };

    /**
     * The associativity of infix operators. They can either be
     * left, right or non associative, or variadic operators treated as one
     * operation, or multiple comparison operators.
     */
    enum InfixAssoc
    {
      /**
       * Left associative.
       */
      ASSOC_LEFT,

      /**
       * Right associative.
       */
      ASSOC_RIGHT,

      /**
       * Non associative.
       */
      ASSOC_NON,

      /**
       * Variadic.
       */
      ASSOC_VARIABLE,

      /**
       * Multiple comparison.
       */
      ASSOC_COMPARISON
    };

    /**
     * A binary operator. Represents a single binary operator and its
     * symbol, mapped to operation, associativity and precedence.
     */
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

      /**
       * The operation.
       */
      std::u32string op;

      /**
       * The symbol that is parsed.
       */
      std::u32string symbol;

      /**
       * The associativity.
       */
      InfixAssoc assoc;

      /**
       * The precedence.
       */
      mpz_class precedence;
    };

    /**
     * Nothing. An nothing node in the AST, it is an error to have a node
     * of this type, but necessary for functioning of the parser.
     */
    struct nil
    {
    };

    /**
     * A constant expression node. The representation of type<value> 
     * after being parsed.
     */
    struct ConstantExpr
    {
      ConstantExpr() = default;

      /**
       * ConstantExpr constructor.
       * @param type type name.
       * @param text the text in the <>.
       */
      ConstantExpr(const u32string& type, const u32string& text)
      : type(type), text(text)
      {}

      u32string type;
      u32string text;
    };

    /**
     * A dimension. An identifier that has been declared in the header as a
     * dimension will be parsed to DimensionExpr.
     */
    struct DimensionExpr
    {
      DimensionExpr() = default;

      DimensionExpr(const u32string& text)
      : text(text)
      {}

      u32string text;
    };

    /**
     * An identifier. Any identifier that isn't a dimension.
     */
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

    /**
     * A unary operation. The operation and operand of a unary operation
     * expression.
     */
    struct UnaryOpExpr
    {
      UnaryOpExpr() = default;

      UnaryOpExpr(const UnaryOperator& o, const Expr& e)
      : op(o), e(e)
      {}

      UnaryOperator op;
      Expr e;
    };

    /**
     * A binary operation. The operation and operands of a binary operation
     * expression.
     */
    struct BinaryOpExpr
    {
      BinaryOpExpr() = default;

      /**
       * Constructs a BinaryOpExpr.
       * @param o The operation.
       * @param l The left hand side expression.
       * @param r The right hand side expression.
       */
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

      /**
       * Adds rhs into the right hand side expression of the current
       * BinaryOpExpr. Used by the variable precedence tree builder
       * when a right associative operator is encountered.
       */
      void
      add_right(const BinaryOperator& op, Expr& rhs);

      /**
       * Set the right hand side expression of a binary operation. Used by
       * the variable precedence tree builder to add an expression onto the
       * right hand side of an operation.
       */
      void
      add_leaf(Expr& e);
    };

    /**
     * Creates a binary operation from the op and the left and right hand
     * sides. Inserts one into the tree of the other depending on the
     * precedence and associativity to create the correct tree for evaluation.
     * @param info The binary operation.
     * @param lhs The left hand side expression.
     * @param rhs The right hand side expression.
     * @return The resulting operation.
     */
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

    struct AtExpr
    {
      AtExpr() = default;

      AtExpr(const Expr& lhs, const Expr& rhs, bool absolute)
      : lhs(lhs), rhs(rhs), absolute(absolute)
      {}

      Expr lhs;
      Expr rhs;
      bool absolute;
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
