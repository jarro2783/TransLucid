#ifndef AST_HPP_INCLUDED
#define AST_HPP_INCLUDED

#include <boost/variant.hpp>
#include <tl/types.hpp>
#include <gmpxx.h>

namespace TransLucid
{
  namespace Tree
  {
    enum InfixAssoc
    {
      ASSOC_LEFT,
      ASSOC_RIGHT,
      ASSOC_NON,
      ASSOC_VARIABLE,
      ASSOC_COMPARISON
    };

    struct BinaryOperation
    {
      BinaryOperation() = default;

      BinaryOperation
      (
        InfixAssoc assoc,
        const std::u32string& op,
        const std::u32string& symbol,
        const mpz_class& precedence
      )
      : op(op), symbol(symbol), assoc(assoc), precedence(precedence)
      {}

      bool
      operator==(const BinaryOperation& rhs) const
      {
        return op == rhs.op && symbol == rhs.symbol &&
        assoc == rhs.assoc && precedence == rhs.precedence;
      }

      bool
      operator!=(const BinaryOperation& rhs) const
      {
        return !(*this == rhs);
      }

      std::u32string op;
      std::u32string symbol;
      InfixAssoc assoc;
      mpz_class precedence;
    };

    enum UnaryType
    {
      UNARY_PREFIX,
      UNARY_POSTFIX
    };

    struct UnaryOperation
    {
      UnaryOperation() = default;

      UnaryOperation
      (
        const u32string& op,
        const u32string& symbol,
        UnaryType type)
      : op(op), symbol(symbol), type(type)
      {}

      u32string op;
      u32string symbol;
      UnaryType type;

      bool
      operator==(const UnaryOperation& rhs) const
      {
        return op == rhs.op && symbol == rhs.symbol && type == rhs.type;
      }
    };

    class AtExpr;
    class BinaryOpExpr;
    class BooleanExpr;
    class BuildTupleExpr;
    class ConstantExpr;
    class DimensionExpr;
    class HashExpr;
    class IdentExpr;
    class IfExpr;
    class OpExpr;
    class PairExpr;
    class RangeExpr;
    class UnaryExpr;

    struct ConstantExpr
    {
      u32string type;
      u32string text;
    };

    struct DimensionExpr
    {
      u32string text;
    };

    struct IdentExpr
    {
      u32string text;
    };

    typedef boost::variant
    <
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
    > Expr;

    struct AtExpr
    {
      Expr lhs;
      Expr rhs;
    };

    struct BinaryOpExpr
    {
      BinaryOperation op;
      Expr lhs;
      Expr rhs;
    };

    struct BuildTupleExpr
    {
      std::vector<std::pair<Expr, Expr>> pairs;
    };

    struct HashExpr
    {
      Expr e;
    };

    struct IfExpr
    {
      Expr condition;
      Expr then;
      std::vector<std::pair<Expr, Expr>> else_ifs;
      Expr else_;
    };

    struct UnaryExpr
    {
      UnaryOperation op;
      Expr e;
    };
  }
}

#endif // AST_HPP_INCLUDED
