/* Prints a Tree::Expr
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
#include <tl/charset.hpp>
#include <tl/utility.hpp>

#include <boost/spirit/home/phoenix/bind/bind_function.hpp>
#include <boost/spirit/home/phoenix/bind/bind_member_function.hpp>
#include <boost/spirit/home/phoenix/object/construct.hpp>
#include <boost/spirit/home/phoenix/object/dynamic_cast.hpp>
#include <boost/spirit/home/phoenix/operator/comparison.hpp>
#include <boost/spirit/home/phoenix/operator/self.hpp>
#include <boost/spirit/home/phoenix/statement/sequence.hpp>
#include <boost/spirit/home/phoenix/statement/if.hpp>
#include <boost/spirit/include/karma.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_function.hpp>
#include <boost/spirit/include/phoenix_stl.hpp>
#include <boost/spirit/include/phoenix_fusion.hpp>

#include <boost/fusion/include/adapt_struct.hpp>

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
        uchar   = karma::string
        [
          _1 = bind(&utf32_to_utf8, construct<u32string>(1, _val))
        ];
        ustring = karma::string[_1 = bind(&utf32_to_utf8, _val)];

        constant =
           karma::string[_1 = bind(&utf32_to_utf8, ph::at_c<0>(_val))]
        << literal('<')
        << karma::string[_1 = bind(&utf32_to_utf8, ph::at_c<1>(_val))]
        << literal('>')
        ;
        dimension = ustring[_1 = at_c<0>(_val)];
        ident = ustring[_1 = at_c<0>(_val)];

        binary_symbol = ustring[_1 = at_c<1>(_val)];
        binary %= ('(') << expr << binary_symbol << expr 
        << literal(')');

        paren_expr = literal('(') << expr << ')';

        hash_expr = ("(#") << expr[_1 = at_c<0>(_val)] << (')');
        pairs %= (expr << ":" << expr) % ", ";
        tuple = literal('[') << pairs[_1 = ph::at_c<0>(_val)] << literal(']');
        at_expr = literal('(') << expr << literal('@') << expr << literal(')');

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
      karma::rule<Iterator, char32_t()> uchar;
      karma::rule<Iterator, Tree::ConstantExpr()> constant;
      karma::rule<Iterator, Tree::DimensionExpr()> dimension;
      karma::rule<Iterator, Tree::IdentExpr()> ident;
      karma::rule<Iterator, Tree::ParenExpr()> paren_expr;
      karma::rule<Iterator, Tree::BinaryOperator()> binary_symbol;
      karma::rule<Iterator, Tree::BinaryOpExpr()> binary;
      karma::rule<Iterator, Tree::HashExpr()> hash_expr;
      karma::rule<Iterator, Tree::TupleExpr()> tuple;
      karma::rule<Iterator, Tree::AtExpr()> at_expr;

      karma::rule
      <
        Iterator,
        Tree::TupleExpr::TuplePairs()
      > pairs;
      std::map<Special::Value, std::string> special_map;
    };
  }
}

#endif // TREE_PRINTER_HPP_INCLUDED
