/* Prints a Tree::Expr
   Copyright (C) 2011 Jarryd Beck and John Plaice

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

#include <tl/tree_printer.hpp>

#include <tl/ast.hpp>
#include <tl/charset.hpp>
#include <tl/parser_util.hpp>
#include <tl/utility.hpp>

#include <boost/spirit/home/phoenix/bind/bind_function.hpp>
#include <boost/spirit/home/phoenix/bind/bind_member_function.hpp>
#include <boost/spirit/home/phoenix/object/construct.hpp>
#include <boost/spirit/home/phoenix/object/dynamic_cast.hpp>
#include <boost/spirit/home/phoenix/operator/comparison.hpp>
#include <boost/spirit/home/phoenix/operator/self.hpp>
#include <boost/spirit/home/phoenix/statement/sequence.hpp>
#include <boost/spirit/home/phoenix/statement/if.hpp>

#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_function.hpp>
#include <boost/spirit/include/phoenix_fusion.hpp>
#include <boost/spirit/include/phoenix_stl.hpp>

#include <boost/spirit/include/karma.hpp>

#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/fusion/include/std_pair.hpp>

BOOST_FUSION_ADAPT_STRUCT
(
  TransLucid::Tree::LiteralExpr,
  (std::u32string, type)
  (std::u32string, text)
)

BOOST_FUSION_ADAPT_STRUCT
(
  TransLucid::Tree::DimensionExpr,
  (TransLucid::u32string, text)
)

BOOST_FUSION_ADAPT_STRUCT
(
  TransLucid::Tree::IdentExpr,
  (TransLucid::u32string, text)
)

typedef std::vector<std::pair<TransLucid::Tree::Expr, TransLucid::Tree::Expr>>
  ExprPairVec;

BOOST_FUSION_ADAPT_STRUCT
(
  TransLucid::Tree::IfExpr,
  (TransLucid::Tree::Expr, condition)
  (TransLucid::Tree::Expr, then)
  (ExprPairVec, else_ifs)
  (TransLucid::Tree::Expr, else_)
)

BOOST_FUSION_ADAPT_STRUCT
(
  TransLucid::Tree::BinaryOperator,
  (TransLucid::u32string, op)
  (TransLucid::u32string, symbol)
  (TransLucid::Tree::InfixAssoc, assoc)
  (mpz_class, precedence)
)

BOOST_FUSION_ADAPT_STRUCT
(
  TransLucid::Tree::ParenExpr,
  (TransLucid::Tree::Expr, e)
)

BOOST_FUSION_ADAPT_STRUCT
(
  TransLucid::Tree::BinaryOpExpr,
  (TransLucid::Tree::Expr, lhs)
  (TransLucid::Tree::BinaryOperator, op)
  (TransLucid::Tree::Expr, rhs)
)

BOOST_FUSION_ADAPT_STRUCT
(
  TransLucid::Tree::HashExpr,
  (TransLucid::Tree::Expr, e)
)

BOOST_FUSION_ADAPT_STRUCT
(
  TransLucid::Tree::TupleExpr,
  (TransLucid::Tree::TupleExpr::TuplePairs, pairs)
);

BOOST_FUSION_ADAPT_STRUCT
(
  TransLucid::Tree::AtExpr,
  (TransLucid::Tree::Expr, lhs)
  (TransLucid::Tree::Expr, rhs)
)

BOOST_FUSION_ADAPT_STRUCT
(
  TransLucid::Tree::LambdaExpr,
  (TransLucid::u32string, name)
  (TransLucid::Tree::Expr, rhs)
)

BOOST_FUSION_ADAPT_STRUCT
(
  TransLucid::Tree::LambdaAppExpr,
  (TransLucid::Tree::Expr, lhs)
  (TransLucid::Tree::Expr, rhs)
)

BOOST_FUSION_ADAPT_STRUCT
(
  TransLucid::Tree::PhiExpr,
  (TransLucid::u32string, name)
  (TransLucid::Tree::Expr, rhs)
)

BOOST_FUSION_ADAPT_STRUCT
(
  TransLucid::Tree::PhiAppExpr,
  (TransLucid::Tree::Expr, lhs)
  (TransLucid::Tree::Expr, rhs)
)

BOOST_FUSION_ADAPT_STRUCT
(
  TransLucid::Tree::BangOpExpr,
  (TransLucid::Tree::Expr, name)
  (std::vector<TransLucid::Tree::Expr>, args)
)

BOOST_FUSION_ADAPT_STRUCT
(
  TransLucid::Tree::WhereExpr,
  (TransLucid::Tree::Expr, e)
  (TransLucid::Tree::WhereExpr::DimensionList, dims)
  (std::vector<TransLucid::Parser::Equation>, vars)
)

template <size_t N>
struct get_tuple
{
  template <typename Arg>
  struct result
  {
    typedef typename std::tuple_element<N, Arg>::type& type;
  };

  template <typename Tuple>
  auto
  operator()(const Tuple& t) const
  -> decltype(std::get<N>(t))&
  {
    return std::get<N>(t);
  }
};

namespace TransLucid
{
  namespace Printer
  {
    namespace karma = boost::spirit::karma;
    namespace phoenix = boost::phoenix;
    namespace ph = boost::phoenix;
    using namespace karma::labels;
    using karma::_val;
    namespace fusion = boost::fusion;
    using boost::spirit::_1;
    using namespace phoenix;

    u32string
    print_dimension(const Tree::DimensionExpr& d)
    {
      if (d.text.empty())
      {
        std::ostringstream os;
        os << "hiddendim_" << d.dim;
        return utf8_to_utf32(os.str());
      }
      else
      {
        return d.text;
      }
    }

    class ExprPrecedence
    {
      public:
      ExprPrecedence(int a)
      : m_a(a), m_b(0)
      {
      }

      ExprPrecedence(int a, const mpz_class& b)
      : m_a(a), m_b(b)
      {
      }

      bool
      operator<(const ExprPrecedence& rhs) const
      {
        return m_a < rhs.m_a || (m_a == rhs.m_a && m_b < rhs.m_b);
      }

      private:
      int m_a;

      //this allows an infinite number of precedences inside the same
      //m_a precedence
      //although more practically it allows binary operators to work
      mpz_class m_b;
    };

    struct print_paren_impl
    {
      template <typename Arg0, typename Arg1, typename Arg2>
      struct result
      {
        typedef std::string type;
      };

      std::string
      operator()
      (
        const ExprPrecedence& parent, 
        const ExprPrecedence& mine, 
        char paren
      ) const
      {
        if (mine < parent)
        {
          return std::string(0, paren);
        }
        return std::string();
      }
    };

    ph::function<print_paren_impl> print_paren;

    template <typename Iterator>
    struct ExprPrinter : karma::grammar<Iterator, Tree::Expr()>
    {
      enum Precedence
      {
        MINUS_INF,
        WHERE_CLAUSE,
        FN_ABSTRACTION,
        BINARY_FN,
        FN_APP,
        PREFIX_FN,
        POSTFIX_FN
      };

      ExprPrinter()
      : ExprPrinter::base_type(expr_top),
      special_map
      {
        {SP_ERROR, "sperror"},
        {SP_ACCESS, "spaccess"},
        {SP_TYPEERROR, "sptype"},
        {SP_DIMENSION, "spdim"},
        {SP_UNDEF, "spundef"},
        {SP_CONST, "spconst"},
        {SP_LOOP, "sploop"}
      }
      {
        //to output, or not to output, that is the question
        paren = karma::string
        [
          _1 = print_paren(_r1, _r2, _r3)
        ];

        //all the expressions
        nil = karma::omit[nildummy] << "nil";

        special = karma::string
        [
          _1 = ph::bind(&ExprPrinter<Iterator>::getSpecial, this, _val)
        ]
        ;

        integer = karma::stream;

        uchar   = karma::string
        [
          _1 = ph::bind(&utf32_to_utf8, construct<u32string>(1, _val))
        ];

        stringLiteral = karma::string[_1 = ph::bind(&utf32_to_utf8, _val)];

        ustring = literal("\"") 
          << karma::string[_1 = ph::bind(&utf32_to_utf8, _val)]
          << literal("\"")
        ;

        constant =
           karma::string[_1 = ph::bind(&utf32_to_utf8, ph::at_c<0>(_val))]
        << literal('<')
        << karma::string[_1 = ph::bind(&utf32_to_utf8, ph::at_c<1>(_val))]
        << literal('>')
        ;

        dimension = stringLiteral[_1 = ph::bind(print_dimension, _val)];

        ident = stringLiteral[_1 = at_c<0>(_val)];

        if_expr = literal("if ") << expr(MINUS_INF) 
          << literal(" then ") << expr(MINUS_INF)
          << elsif_list
          << literal(" else ") << expr(MINUS_INF) << literal(" fi ")
        ;

        elsif_list %= *(one_elsif);

        one_elsif = 
          literal(" elsif ") 
          << expr(MINUS_INF)
          << literal(" then ") 
          << expr(MINUS_INF)
        ;

        binary_symbol = stringLiteral[_1 = at_c<1>(_val)];

        binary = 
          paren
          (
            _r1,
            ph::construct<ExprPrecedence>(
              BINARY_FN, ph::at_c<3>(ph::at_c<1>(_val))),
            '('
          )
          //TODO fix this
          << expr(BINARY_FN) << binary_symbol << expr(BINARY_FN)
          << paren
          (
            _r1,
            ph::construct<ExprPrecedence>(BINARY_FN,
              ph::at_c<3>(ph::at_c<1>(_val))),
            ')'
          );

        paren_expr = literal('(') << expr(MINUS_INF) << ')';

        hash_expr = paren(_r1, PREFIX_FN, '(') << literal("#") << 
          expr(PREFIX_FN)[_1 = at_c<0>(_val)] << 
          paren(_r1, PREFIX_FN, ')')
        ;

        pairs %= one_pair % ", ";

        one_pair %= expr(MINUS_INF) << literal(" <- ") << expr(MINUS_INF);
        tuple = literal('[') << pairs[_1 = ph::at_c<0>(_val)] << literal(']');

        at_expr = paren(_r1, FN_APP, '(') 
          << expr(FN_APP)
          << literal(" @ ") 
          << expr(FN_APP)
          << paren(_r1, FN_APP, ')')
        ;

        lambda_function = karma::string(literal("(\\")) 
          << stringLiteral[_1 = ph::at_c<0>(_val)] 
          << literal(" -> ") << expr(MINUS_INF)[_1 = ph::at_c<1>(_val)] 
          << literal(")");

        lambda_application = 
          literal("(") << expr(FN_APP) << literal(".") << expr(FN_APP)
          << literal(")");

        name_function = karma::string(literal("(\\\\")) 
          << stringLiteral[_1 = ph::at_c<0>(_val)] 
          << literal(" -> ") << expr(MINUS_INF)[_1 = ph::at_c<1>(_val)] 
          << literal(")");

        name_application = 
          paren(_r1, FN_APP, '(') <<
          expr(FN_APP) << 
          literal(" ") << 
          expr(FN_APP) << 
          paren(_r1, FN_APP, ')')
        ;

        where = paren(_r1, WHERE_CLAUSE, '(') << expr(WHERE_CLAUSE) 
          << literal(" where\n") << dimlist 
          << varlist << literal("end")
          << paren(_r1, WHERE_CLAUSE, ')')
        ;

        dimlist = *(oneDim);

        oneDim = literal("dim ") << stringLiteral << literal(" <- ") 
          << expr(MINUS_INF) << literal(";;\n");

        varlist = *(eqn);

        eqn = literal("var ") 
          << stringLiteral [_1 = ph::function<get_tuple<0>>()(_val)]
          << literal(" ")
          << expr(MINUS_INF) [_1 = ph::function<get_tuple<1>>()(_val)]
          << literal(" & ") 
          << expr(MINUS_INF) [_1 = ph::function<get_tuple<2>>()(_val)]
          << literal(" = ") 
          << expr(MINUS_INF) [_1 = ph::function<get_tuple<3>>()(_val)]
          << literal(";;\n")
          ;

        bangop = expr(FN_APP) << 
          literal("!(") << *(expr(FN_APP) % literal(","))
          << literal(")");

        // TODO: Missing unary
        expr %=
          nil
        | karma::bool_
        | integer
        | special
        | uchar
        | ustring
        | constant
        | dimension
        | ident
        | paren_expr
        // | unary -- where is it?
        | binary(_r1)
        | hash_expr(_r1)
        | tuple
        | at_expr(_r1)
        | lambda_function
        | lambda_application
        | name_function
        | name_application(_r1)
        | where(_r1)
        | bangop
        | if_expr
        ;

        expr_top %= expr(MINUS_INF);
      }

      const std::string&
      getSpecial(Special v)
      {
        return special_map[v];
      }

      karma::rule<Iterator, Tree::Expr()> expr_top;
      karma::rule<Iterator, Tree::Expr(ExprPrecedence)> expr;

      karma::rule<Iterator, Tree::nil()> nil;
      karma::rule<Iterator, Tree::nil()> nildummy;
      karma::rule<Iterator, Special()> special;
      karma::rule<Iterator, mpz_class()> integer;
      karma::rule<Iterator, u32string()> ustring;
      karma::rule<Iterator, char32_t()> uchar;
      karma::rule<Iterator, Tree::LiteralExpr()> constant;
      karma::rule<Iterator, Tree::DimensionExpr()> dimension;
      karma::rule<Iterator, Tree::IdentExpr()> ident;
      karma::rule<Iterator, Tree::ParenExpr()> paren_expr;
      karma::rule<Iterator, Tree::IfExpr()> if_expr;
      karma::rule<Iterator, Tree::BinaryOperator()> binary_symbol;
      karma::rule<Iterator, Tree::BinaryOpExpr(ExprPrecedence)> binary;
      karma::rule<Iterator, Tree::HashExpr(ExprPrecedence)> hash_expr;
      karma::rule<Iterator, Tree::TupleExpr()> tuple;
      karma::rule<Iterator, Tree::AtExpr(ExprPrecedence)> at_expr;
      karma::rule<Iterator, Tree::LambdaExpr()> lambda_function;
      karma::rule<Iterator, Tree::LambdaAppExpr()> lambda_application;
      karma::rule<Iterator, Tree::PhiExpr()> name_function;
      karma::rule<Iterator, Tree::PhiAppExpr(ExprPrecedence)> name_application;
      karma::rule<Iterator, Tree::WhereExpr(ExprPrecedence)> where;
      karma::rule<Iterator, Tree::BangOpExpr()> bangop;

      karma::rule<Iterator, u32string()> stringLiteral;

      karma::rule
      <
        Iterator,
        void(ExprPrecedence, ExprPrecedence, char)
      > paren;

      karma::rule
      <
        Iterator, 
        std::vector<std::pair<Tree::Expr, Tree::Expr>>()
      > elsif_list;

      karma::rule
      <
        Iterator,
        std::pair<Tree::Expr, Tree::Expr>()
      > one_elsif;

      karma::rule
      <
        Iterator,
        TransLucid::Tree::WhereExpr::DimensionList()
      > dimlist;

      karma::rule
      <
        Iterator,
        std::pair<u32string, Tree::Expr>()
      > oneDim;

      karma::rule
      <
        Iterator,
        std::vector<TransLucid::Parser::Equation>()
      > varlist;

      karma::rule
      <
        Iterator,
        Tree::TupleExpr::TuplePairs()
      > pairs;

      karma::rule
      <
        Iterator,
        std::pair<Tree::Expr, Tree::Expr>()
      > one_pair;

      karma::rule
      <
        Iterator,
        Parser::Equation()
      > eqn;

      std::map<Special, std::string> special_map;
    };
  }

  std::string print_expr_tree(const Tree::Expr& expr)
  {
    typedef std::back_insert_iterator<std::string> out_iter;
    Printer::ExprPrinter<out_iter> print_grammar;
    std::string generated;
    std::back_insert_iterator<std::string> outit(generated);

    Printer::karma::generate(outit, print_grammar, expr);
    return generated;
  }
}
