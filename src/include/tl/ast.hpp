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
    class UnaryOpExpr;

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

    struct nil
    {
    };

    typedef boost::variant
    <
      nil,
      boost::recursive_wrapper<AtExpr>,
      boost::recursive_wrapper<BinaryOpExpr>,
      boost::recursive_wrapper<BuildTupleExpr>,
      ConstantExpr,
      DimensionExpr,
      boost::recursive_wrapper<HashExpr>,
      IdentExpr,
      boost::recursive_wrapper<IfExpr>,
      boost::recursive_wrapper<UnaryOpExpr>,
      bool,
      char32_t, //replaces UcharExpr
      mpz_class, //replaces IntegerExpr
      Special::Value, //replaces SpecialExpr
      u32string //replaces StringExpr
    > Expr;

    struct AtExpr
    {
      AtExpr() = default;

      AtExpr(const Expr& lhs, const Expr& rhs)
      : lhs(lhs), rhs(rhs)
      {}

      Expr lhs;
      Expr rhs;
      bool relative;
    };

    struct BinaryOpExpr
    {
      BinaryOpExpr() = default;

      BinaryOpExpr(BinaryOperation o,
                   const Expr& l,
                   const Expr& r)
      : op(o), lhs(l), rhs(r)
      {}

      BinaryOperation op;
      Expr lhs;
      Expr rhs;

      void
      add_right(const BinaryOperation& op, Expr& rhs);

      void
      add_leaf(Expr& e);
    };

    struct BuildTupleExpr
    {
      typedef
      std::vector<boost::fusion::vector<Expr, Expr>>
      TuplePairs;

      TuplePairs pairs;

      BuildTupleExpr() = default;

      BuildTupleExpr(const TuplePairs& p)
      : pairs(p)
      {}
    };

    struct HashExpr
    {
      HashExpr() = default;

      HashExpr(Expr e)
      : e(e)
      {}

      Expr e;
    };

    struct IfExpr
    {
      IfExpr() = default;

      template <typename List>
      IfExpr(Expr c,
             Expr t,
             const List& eif,
             Expr e)
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

    struct UnaryOpExpr
    {
      UnaryOpExpr() = default;

      UnaryOpExpr(const UnaryOperation& o, const Expr& e)
      : op(o), e(e)
      {}

      UnaryOperation op;
      Expr e;
    };

    Expr insert_binary_operation
    (
      const BinaryOperation& info,
      Expr& lhs,
      Expr& rhs
    );

    #define PRINT_NODE(n) \
    inline \
    std::ostream& operator<<(std::ostream& os, const n &) \
    { \
       os << #n ; \
       return os; \
    } \

    PRINT_NODE(nil)
    PRINT_NODE(AtExpr)
    PRINT_NODE(BinaryOpExpr)
    PRINT_NODE(BuildTupleExpr)
    PRINT_NODE(ConstantExpr)
    PRINT_NODE(DimensionExpr)
    PRINT_NODE(HashExpr)
    PRINT_NODE(IdentExpr)
    PRINT_NODE(IfExpr)
    PRINT_NODE(UnaryOpExpr)

#if 0
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
    class UnaryOpExpr;
#endif
  }
}

#endif // AST_HPP_INCLUDED
