/* TODO: Give a descriptor.
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
#include <boost/spirit/home/phoenix/bind/bind_member_function.hpp>
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
  TransLucid::Tree::DimensionExpr,
  (TransLucid::u32string, text)
)

BOOST_FUSION_ADAPT_STRUCT
(
  TransLucid::Tree::IdentExpr,
  (TransLucid::u32string, text)
)

BOOST_FUSION_ADAPT_STRUCT
(
  TransLucid::Tree::BinaryOperation,
  (TransLucid::u32string, symbol)
)

BOOST_FUSION_ADAPT_STRUCT
(
  TransLucid::Tree::BinaryOpExpr,
  (TransLucid::Tree::Expr, lhs)
  (TransLucid::Tree::BinaryOperation, op)
  (TransLucid::Tree::Expr, rhs)
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

BOOST_FUSION_ADAPT_STRUCT
(
  TransLucid::Tree::AtExpr,
  (TransLucid::Tree::Expr, lhs)
  (TransLucid::Tree::Expr, rhs)
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
      ExprPrinter()
      : ExprPrinter::base_type(expr),
      special_map
      {
        {Special::ERROR, "sperror"},
        {Special::ACCESS, "spaccess"},
        {Special::TYPEERROR, "sptype"},
        {Special::DIMENSION, "spdim"},
        {Special::UNDEF, "spundef"},
        {Special::CONST, "spconst"},
        {Special::LOOP, "sploop"}
      }
      {
        nil = karma::omit[nildummy] << "nil";
        special = karma::string
        [
          _1 = ph::bind(&ExprPrinter<Iterator>::getSpecial, this, _val)
        ]
        ;
        integer = karma::stream;
        ustring = karma::string[_1 = bind(&utf32_to_utf8, _val)];

        constant =
           karma::string[_1 = bind(&utf32_to_utf8, ph::at_c<0>(_val))]
        << '<'
        << karma::string[_1 = bind(&utf32_to_utf8, ph::at_c<1>(_val))]
        << '>'
        ;
        dimension = ustring[_1 = at_c<0>(_val)];
        ident = ustring[_1 = at_c<0>(_val)];

        binary_symbol = ustring[_1 = at_c<0>(_val)];
        binary %= '(' << expr << binary_symbol << expr << ')';

        hash_expr = "(#" << expr[_1 = at_c<0>(_val)] << ')';
        pairs %= (expr << ":" << expr) % ", ";
        tuple = '[' << pairs[_1 = ph::at_c<0>(_val)] << ']';
        at_expr = '(' << expr << '@' << expr << ')';

        // TODO: Missing uchar and unary
        expr %=
          nil
        | karma::bool_
        | integer
        | special
        // | uchar -- where is it?
        | ustring
        | constant
        | dimension
        | ident
        // | unary -- where is it?
        | binary
        | hash_expr
        | tuple
        | at_expr
        ;
      }

      const std::string&
      getSpecial(Special::Value v)
      {
        return special_map[v];
      }

      karma::rule<Iterator, Tree::Expr()> expr;

      karma::rule<Iterator, Tree::nil()> nil;
      karma::rule<Iterator, Tree::nil()> nildummy;
      karma::rule<Iterator, Special::Value()> special;
      karma::rule<Iterator, mpz_class()> integer;
      karma::rule<Iterator, u32string()> ustring;
      karma::rule<Iterator, Tree::ConstantExpr()> constant;
      karma::rule<Iterator, Tree::DimensionExpr()> dimension;
      karma::rule<Iterator, Tree::IdentExpr()> ident;
      karma::rule<Iterator, Tree::BinaryOperation()> binary_symbol;
      karma::rule<Iterator, Tree::BinaryOpExpr()> binary;
      karma::rule<Iterator, Tree::HashExpr()> hash_expr;
      karma::rule<Iterator, Tree::BuildTupleExpr()> tuple;
      karma::rule<Iterator, Tree::AtExpr()> at_expr;

      karma::rule
      <
        Iterator,
        Tree::BuildTupleExpr::TuplePairs()
      > pairs;
      std::map<Special::Value, std::string> special_map;
    };
  }
}

#endif // TREE_PRINTER_HPP_INCLUDED
