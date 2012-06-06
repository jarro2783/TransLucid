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

#ifndef AST_NEW_HPP_INCLUDED
#define AST_NEW_HPP_INCLUDED

#ifdef DEBUG
#include <iostream>
#endif

#include <vector>

#include <gmpxx.h>

#include <tl/ast_fwd.hpp>
#include <tl/types_basic.hpp>
#include <tl/variant.hpp>

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
      UNARY_PREFIX, /**< prefix operator */
      UNARY_POSTFIX /**< postfix operator */
    };

    /**
     * A parsed unary operator. Stores the symbol and the operator which
     * it maps to.
     */
    struct UnaryOperator
    {
      UnaryOperator() = default;

      /**
       * Construct a unary operator. 
       * @param op The name of the operation.
       * @param symbol The symbol that maps to the operation.
       * @param type The type of operation, postfix or prefix.
       */
      UnaryOperator
      (
        const u32string& op,
        const u32string& symbol,
        UnaryType type,
        bool cbn = false
      )
      : op(op), symbol(symbol), type(type), call_by_name(cbn)
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

      //call by value if false
      bool call_by_name;

      /**
       * Equality of two unary operations.
       */
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
      ASSOC_LEFT, /**< Left associative.*/
      ASSOC_RIGHT, /**< Right associative.*/
      ASSOC_NON, /**< Non associative.*/
      ASSOC_VARIABLE, /**< Variadic.*/
      ASSOC_COMPARISON /**< Multiple comparison.*/
    };

    /**
     * A binary operator. Represents a single binary operator and its
     * symbol, mapped to operation, associativity and precedence.
     */
    struct BinaryOperator
    {
      BinaryOperator() = default;

      /**
       * Construct a binary operator.
       * @param assoc The association.
       * @param op The name of the operation.
       * @param symbol The symbol that maps to this operation.
       * @param precedence The precedence of the operator.
       */
      BinaryOperator
      (
        InfixAssoc assoc,
        const u32string& op,
        const u32string& symbol,
        const mpz_class& precedence,
        bool cbn = false
      )
      : op(op), symbol(symbol), assoc(assoc), precedence(precedence), cbn(cbn)
      {}

      /**
       * Determine equality of two binary operators. They are equal if
       * all the members are equal.
       * @param rhs The other operator to compare to.
       * @return All the members of *this equal to all the members of rhs.
       */
      bool
      operator==(const BinaryOperator& rhs) const
      {
        return op == rhs.op && symbol == rhs.symbol &&
        assoc == rhs.assoc && precedence == rhs.precedence;
      }

      /**
       * Determines if two binary operators are not equal.
       * @param rhs The other operator to compare to.
       * @return !(*this == rhs)
       */
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

      /**
       * Call-by-name or call-by-value. True for call-by-name.
       */
      bool cbn;
    };

    /**
     * Nothing. A nothing node in the AST, this will occur whenever there
     * is no tree for a particular entity.
     */ 
    struct nil
    {
    };

    /**
     * A constant expression node. The representation of type<value> 
     * after being parsed.
     */
    struct LiteralExpr
    {
      LiteralExpr() = default;

      /**
       * LiteralExpr constructor.
       * @param type type name.
       * @param text the text in the <>.
       */
      LiteralExpr(const u32string& type, const u32string& text)
      : type(type), text(text)
      {}

      /**
       * Construct a LiteralExpr from a pair.
       * @param value The pair representing the constant.
       */
      LiteralExpr(const std::pair<u32string, u32string>& value)
      : type(value.first), text(value.second)
      {
      }

      u32string type; /**< The typename.*/
      u32string text; /**< The text in the angle brackets.*/
    };

    /**
     * A dimension. An identifier that has been declared in the header as a
     * dimension will be parsed to DimensionExpr.
     */
    struct DimensionExpr
    {
      DimensionExpr() = default;

      /**
       * Construct a dimension expression.
       * @param text The name of the dimension.
       */
      DimensionExpr(const u32string& text)
      : text(text)
      {}

      DimensionExpr(dimension_index dim)
      : dim(dim) {}

      u32string text; /**< The name of the dimension.*/

      /** This is a bit of a hack.
       * If the text isn't set, use the dimension index instead.
       */
      dimension_index dim;
    };

    /**
     * An identifier. Any identifier that isn't a dimension.
     */
    struct IdentExpr
    {
      IdentExpr() = default;

      /**
       * Construct an identifier expression.
       * @param t The name of the identifier.
       */
      IdentExpr(const u32string& t)
      : text(t)
      {}

      u32string text; /**< The name of the identifier.*/
    };

    struct BaseAbstractionExpr
    {
      BaseAbstractionExpr() = default;

      BaseAbstractionExpr(const u32string& name)
      : name(name)
      {}

      u32string name;
    };

    struct HashSymbol
    {
    };

    //defined in ast_fwd.hpp
    /**
     * Abstract syntax tree node. A single expression node in the 
     * abstract syntax tree which is created by the parser.
     */
    #if 0
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
      BaseAbstraction,
      recursive_wrapper<ParenExpr>,
      recursive_wrapper<UnaryOpExpr>,
      recursive_wrapper<BinaryOpExpr>,
      recursive_wrapper<IfExpr>,
      recursive_wrapper<HashExpr>,
      recursive_wrapper<TupleExpr>,
      recursive_wrapper<AtExpr>,
      recursive_wrapper<LambdaExpr>,
      recursive_wrapper<PhiExpr>,
      recursive_wrapper<BangAppExpr>,
      recursive_wrapper<LambdaAppExpr>,
      recursive_wrapper<PhiAppExpr>,
      recursive_wrapper<WhereExpr>
    > Expr;
    #endif

    /**
     * A parenthesised expression.
     */
    struct ParenExpr
    {
      ParenExpr() = default;

      /**
       * Construct a parenthesised expression.
       * @param e The inside expression.
       */
      explicit ParenExpr(const Expr& e)
      : e(e)
      {
      }

      Expr e; /**<The expression.*/
    };

    /**
     * A unary operation. The operation and operand of a unary operation
     * expression.
     */
    struct UnaryOpExpr
    {
      UnaryOpExpr() = default;

      /**
       * Construct a unary operation expression.
       * @param o The type of unary operation.
       * @param e The expression to operate on.
       */
      UnaryOpExpr(const UnaryOperator& o, const Expr& e)
      : op(o), e(e)
      {}

      UnaryOperator op; /**< The type of unary operation.*/
      Expr e; /**< The expression to operate on.*/
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

      BinaryOperator op; /**< The binary operation.*/
      Expr lhs; /**< The left hand side expression.*/
      Expr rhs; /**< The right hand side expression.*/

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

    struct MakeIntenExpr
    {
      MakeIntenExpr() = default;

      MakeIntenExpr(Expr e)
      : expr(e)
      {}

      MakeIntenExpr(Expr e, std::vector<dimension_index> scope)
      : expr(e), scope(scope)
      {}

      Expr expr;

      std::vector<dimension_index> scope;
    };

    struct EvalIntenExpr
    {
      EvalIntenExpr() = default;

      EvalIntenExpr(const Tree::Expr& rhs)
      : expr(rhs)
      {
      }

      Expr expr;
    };

    /**
     * Host operation. Evaluates an operation that has been provided
     * in the host environment. It is strict and there is no partial
     * application.
     */
    struct BangAppExpr
    {
      BangAppExpr() = default;

      /**
       * Construct a bang operation node.
       * @param name The expression that will return the name.
       * @param args A vector of the arguments.
       */
      BangAppExpr(const Expr& name, const std::vector<Expr>& args)
      : name(name), args(args)
      {
      }

      BangAppExpr(const Expr& lhs, const Expr& rhs)
      : name(lhs)
      , args({rhs})
      {
      }

      Expr name; /**< The operation name. */
      std::vector<Expr> args; /**< The arguments. */
    };

    /**
     * An if expression. Represents if e1 then elsif e2 else fi.
     */
    struct IfExpr
    {
      IfExpr() = default;

      IfExpr
      (
        const Expr& c,
        const Expr& t,
        const std::vector<std::pair<Expr, Expr>>& eif,
        const Expr& e
      )
      : condition(c)
      , then(t)
      , else_ifs(eif)
      , else_(e)
      {
      }

      Expr condition; /**< The condition expression. */
      Expr then; /**< The if true expression. */
      /**
       * Elsif condition/expression pairs. The conditions should evaluate to
       * booleans, if they are true the expression is the result.
       */
      std::vector<std::pair<Expr, Expr>> else_ifs;
      Expr else_; /**< The else expression.*/
    };

    /**
     * Hash expression. This is \#E.
     */
    struct HashExpr
    {
      HashExpr() = default;

      /**
       * Construct a hash expression.
       * @param e The sub expression.
       */
      explicit HashExpr(const Expr& e, bool cached = true)
      : e(e), cached(cached)
      {}

      /**
       * The sub expression.
       */
      Expr e;

      /**
       * Is the resulting dimension being considered for caching?
       * If not, don't look at delta.
       */
      bool cached;
    };

    /**
     * A tuple building expression. These are of the form
     * [E11:E12, ..., En1:En2].
     */
    struct TupleExpr
    {
      /**
       * The data structure which holds the pairs.
       */
      typedef
      std::vector<std::pair<Expr, Expr>>
      TuplePairs;

      /**
       * All of the pairs in the tuple.
       */
      TuplePairs pairs;

      TupleExpr() = default;

      /**
       * Construct a TupleExpr.
       * @param p The pairs of expressions.
       */
      TupleExpr(const TuplePairs& p)
      : pairs(p)
      {}
    };

    struct BestofExpr
    {
      BestofExpr() = default;

      //region, boolean, expr
      std::vector<std::tuple<Expr, Expr, Expr>> expressions;

      u32string name;
    };

    /**
     * A context change expression. E1 @ E2, evaluate E1 with the tuple
     * returned by E2 as the new context.
     */
    struct AtExpr
    {
      AtExpr() = default;

      /**
       * Construct an AtExpr. Takes as parameters the subexpressions and
       * whether it is a relative or absolute context change.
       * @param lhs The left hand side expression.
       * @param rhs The right hand side expression.
       */
      AtExpr(const Expr& lhs, const Expr& rhs)
      : lhs(lhs), rhs(rhs)
      {}

      /**
       * The left hand side expression.
       */
      Expr lhs;

      /**
       * The right hand side expression.
       */
      Expr rhs;
    };

    /**
     * A lambda expression. An expression node representing a lambda 
     * expression which creates an unnamed function.
     */
    struct LambdaExpr
    {
      LambdaExpr() 
      : argDim(0)
      {}

      /**
       * Construct a LambdaExpr.
       * @param name The parameter to bind.
       * @param rhs The right-hand-side expression.
       */
      template <typename RExpr>
      LambdaExpr
      (
        const u32string& name, 
        RExpr&& rhs
      )
      : name(name), rhs(std::forward<RExpr>(rhs)), argDim(0)
      {
      }

      u32string name; /**<The bound parameter.*/
      Expr rhs; /**<The right-hand-side expression.*/

      dimension_index argDim;
      std::vector<dimension_index> scope;

      //the free variables to evaluate
      std::vector<std::pair<u32string, dimension_index>> free;
    };

    //TODO: fix TreeToWSTree when I implement this
    struct PhiExpr
    {
      PhiExpr()
      : argDim(0)
      {}

      PhiExpr(const u32string& name, const Expr& rhs)
      : name(name), rhs(rhs), argDim(0)
      {
      }

      u32string name;
      //std::vector<Expr> binds;
      Expr rhs;

      dimension_index argDim;
      std::vector<dimension_index> scope;

      //the free variables to evaluate
      std::vector<std::pair<u32string, dimension_index>> free;
    };

    /**
     * By value function application expression. An expression node
     * representing by value function application.
     */
    struct LambdaAppExpr
    {
      LambdaAppExpr() = default;

      /**
       * Construct a by value application expression.
       * @param lhs The left-hand-side expression.
       * @param rhs The right-hand-side expression.
       */
      LambdaAppExpr(const Expr& lhs, const Expr& rhs)
      : lhs(lhs), rhs(rhs)
      {
      }

      Expr lhs; /**<The lhs.*/
      Expr rhs; /**<The rhs.*/
    };

    /**
     * Named function application expression. An expression node representing
     * named function application.
     */
    struct PhiAppExpr
    {
      PhiAppExpr() = default;

      /**
       * Construct a named application expression.
       * @param lhs The left-hand-side expression.
       * @param rhs The right-hand-side expression.
       */
      PhiAppExpr(const Expr& lhs, const Expr& rhs)
      : lhs(lhs), rhs(rhs)
      {
      }

      PhiAppExpr(const Expr& lhs, const Expr& rhs, 
        const std::vector<dimension_index>& lall)
      : lhs(lhs), rhs(rhs), Lall(lall)
      {
      }

      Expr lhs; /**<The lhs expression.*/
      Expr rhs; /**<The rhs expression.*/

      //the Lall of the rhs
      std::vector<dimension_index> Lall;
    };

    struct WhereExpr
    {
      Expr e;

      u32string name;

      typedef std::vector<std::pair<u32string, Expr>> DimensionList;
      DimensionList dims;

      //redefinition of Equation because otherwise the definition would be
      //circular
      std::vector
      <
        std::tuple<u32string, Expr, Expr, Expr>
      > vars;

      dimension_index myDim;
      std::vector<dimension_index> Lin;
      std::vector<dimension_index> Lout;
      std::vector<size_t> whichDims;
    };

    struct ConditionalBestfitExpr
    {
      //provenance [...] | boolean = expr
      typedef std::tuple<int, Expr, Expr, Expr> Declaration;

      std::vector<Declaration> declarations;
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

    #ifdef DEBUG
    #define PRINT_NODE(n) \
    inline \
    std::ostream& operator<<(std::ostream& os, const n &) \
    { \
       os << #n ; \
       return os; \
    } \

    PRINT_NODE(nil)
    PRINT_NODE(LiteralExpr)
    PRINT_NODE(DimensionExpr)
    PRINT_NODE(IdentExpr)
    PRINT_NODE(ParenExpr)
    PRINT_NODE(UnaryOpExpr)
    PRINT_NODE(BinaryOpExpr)
    PRINT_NODE(IfExpr)
    PRINT_NODE(HashSymbol)
    PRINT_NODE(HashExpr)
    PRINT_NODE(TupleExpr)
    PRINT_NODE(AtExpr)
    PRINT_NODE(LambdaExpr)
    PRINT_NODE(PhiExpr)
    PRINT_NODE(BangAppExpr)
    PRINT_NODE(LambdaAppExpr)
    PRINT_NODE(PhiAppExpr)
    PRINT_NODE(WhereExpr)
    PRINT_NODE(ConditionalBestfitExpr)
    #endif
  }

  namespace Parser
  {
    /**
     * A parsed equation.
     * The tuple is defined as: name, [], & bool, Expr
     */
    typedef std::tuple<u32string, Tree::Expr, Tree::Expr, Tree::Expr>
    Equation;
  }
}

#endif // AST_NEW_HPP_INCLUDED
