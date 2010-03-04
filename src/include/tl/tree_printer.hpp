#ifndef TREE_PRINTER_HPP_INCLUDED
#define TREE_PRINTER_HPP_INCLUDED

#include <tl/expr.hpp>

#include <boost/spirit/include/karma.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_function.hpp>
#include <boost/spirit/home/phoenix/operator/self.hpp>
#include <boost/spirit/include/phoenix_stl.hpp>
#include <boost/spirit/include/phoenix_fusion.hpp>
#include <boost/fusion/include/adapt_struct.hpp>
#include <tl/utility.hpp>
#include <boost/spirit/home/phoenix/bind/bind_function.hpp>
#include <boost/spirit/home/phoenix/object/dynamic_cast.hpp>
#include <boost/spirit/home/phoenix/statement/sequence.hpp>
#include <boost/spirit/home/phoenix/statement/if.hpp>
#include <boost/spirit/home/phoenix/operator/comparison.hpp>

BOOST_FUSION_ADAPT_STRUCT
(
  TransLucid::AST::IntegerExpr,
  (mpz_class, m_value)
)

BOOST_FUSION_ADAPT_STRUCT
(
  TransLucid::AST::ConstantExpr,
  (std::u32string, name)
  (std::u32string, value)
)

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

    struct get_integer_impl
    {
      template <typename T1>
      struct result { typedef const mpz_class& type; };

      const mpz_class& operator()(const AST::IntegerExpr* e) const
      {
        return e->m_value;
      }
    };

    phoenix::function<get_integer_impl> get_integer;

    template <typename Iterator>
    struct ExprPrinter : karma::grammar<Iterator, AST::Expr()>
    {
      ExprPrinter() : ExprPrinter::base_type(expr)
      {
        integer = karma::stream[_1 = ph::at_c<0>(_val)];

        constant =
           karma::string[_1 = bind(&utf32_to_utf8, ph::at_c<0>(_val))]
        << '<'
        << karma::string[_1 = bind(&utf32_to_utf8, ph::at_c<1>(_val))]
        << '>'
        ;

        primary %=
          integer
        | constant
        //[
        //  _1 = ph::dynamic_cast_<const AST::IntegerExpr>(_val)
        //]
        ;

        expr %= primary;
      }

      karma::rule<Iterator, AST::Expr()>
        expr
      ;

      karma::rule<Iterator, AST::Expr()>
        primary
      ;

      karma::rule<Iterator, AST::IntegerExpr()> integer;

      karma::rule<Iterator, AST::ConstantExpr()> constant;
    };
  }
}

#endif // TREE_PRINTER_HPP_INCLUDED
