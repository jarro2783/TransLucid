#ifndef TREE_PRINTER_HPP_INCLUDED
#define TREE_PRINTER_HPP_INCLUDED

#include <tl/ast.hpp>

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
  TransLucid::Tree::ConstantExpr,
  (std::u32string, type)
  (std::u32string, text)
)

BOOST_FUSION_ADAPT_STRUCT
(
  TransLucid::Tree::HashExpr,
  (TransLucid::Tree::Expr, e)
)

BOOST_FUSION_ADAPT_STRUCT
(
  TransLucid::Tree::BuildTupleExpr,
  (TransLucid::Tree::BuildTupleExpr::TuplePairs, pairs)
);

//BOOST_FUSION_ADAPT_STRUCT
//(
//  TransLucid::Tree::BuildTupleExpr,
//  (TransLucid::Tree::BuildTupleExpr::TuplePairs, pairs)
//)

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

    #if 0
    struct get_integer_impl
    {
      template <typename T1>
      struct result { typedef const mpz_class& type; };

      const mpz_class& operator()(const Tree::IntegerExpr& e) const
      {
        return e->m_value;
      }
    };

    phoenix::function<get_integer_impl> get_integer;
    #endif

    template <typename Iterator>
    struct ExprPrinter : karma::grammar<Iterator, Tree::Expr()>
    {
      ExprPrinter() : ExprPrinter::base_type(expr)
      {
        integer = karma::stream;

        constant =
           karma::string[_1 = bind(&utf32_to_utf8, ph::at_c<0>(_val))]
        << '<'
        << karma::string[_1 = bind(&utf32_to_utf8, ph::at_c<1>(_val))]
        << '>'
        ;

        hash %= '#' << expr;

        tuple = '[' << pairs[_1 = ph::at_c<0>(_val)] << ']';

        pairs %= (expr << " : " << expr) % ", ";

        expr %=
          integer
        | constant
        | karma::bool_
        | hash
        ;
      }

      karma::rule<Iterator, Tree::Expr()> expr;
      karma::rule<Iterator, mpz_class()> integer;
      karma::rule<Iterator, Tree::ConstantExpr()> constant;
      karma::rule<Iterator, Tree::HashExpr()> hash;
      karma::rule<Iterator, Tree::BuildTupleExpr()> tuple;
      karma::rule
      <
        Iterator,
        Tree::BuildTupleExpr::TuplePairs()
      > pairs;
    };
  }
}

#endif // TREE_PRINTER_HPP_INCLUDED
