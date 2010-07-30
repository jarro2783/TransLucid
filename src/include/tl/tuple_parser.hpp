/* Parses [E11:E12, ..., En1:En2]
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

#ifndef TUPLE_PARSER_HPP_INCLUDED
#define TUPLE_PARSER_HPP_INCLUDED

#include <tl/parser_fwd.hpp>
#include <tl/parser_util.hpp>
#include <boost/spirit/home/phoenix/object/new.hpp>
#include <boost/spirit/home/phoenix/object/construct.hpp>
#include <boost/spirit/home/phoenix/container.hpp>
#include <boost/spirit/include/qi_core.hpp>
#include <tl/ast.hpp>

namespace TransLucid
{
  namespace Parser
  {
    typedef std::vector<boost::fusion::vector<Tree::Expr, Tree::Expr>>
      vector_pair_expr;

    /**
     * Parses a tuple. A tuple is [E:E, ...].
     */
    template <typename Iterator>
    class TupleGrammar
    : public qi::grammar<Iterator, Tree::Expr(), SkipGrammar<Iterator>>
    {
      public:

      TupleGrammar()
      : TupleGrammar::base_type(context_perturb)
      {
        using namespace qi::labels;
        using namespace boost::phoenix;
        namespace phoenix = boost::phoenix;
        //expr = self.parsers.expr_parser.top();

        tuple_inside = pair[push_back(_val, _1)] % ',';

        pair %=
          (
             expr
           >  ':'
           > expr
          )
          //[
          //  _val = construct<boost::fusion::vector<Tree::Expr, Tree::Expr>>
          //         (_1, _2)
          //]
        ;

        context_perturb =
           (
              '['
            > tuple_inside
            > ']'
           )
           [
             _val = construct<Tree::TupleExpr>(_1)
           ]
        ;

        expr.name("expr");
        pair.name("pair");
        context_perturb.name("context_perturb");
        tuple_inside.name("tuple_inside");

        BOOST_SPIRIT_DEBUG_NODE(context_perturb);
        BOOST_SPIRIT_DEBUG_NODE(pair);
        BOOST_SPIRIT_DEBUG_NODE(tuple_inside);
      }

      template <typename T>
      void
      set_expr(const T& t)
      {
        expr %= t;
      }

      private:

      qi::rule<Iterator, Tree::Expr(), SkipGrammar<Iterator>>
        expr,
        context_perturb
      ;

      qi::rule<Iterator, vector_pair_expr(), SkipGrammar<Iterator>>
        tuple_inside
      ;

      qi::rule
      <
        Iterator,
        boost::fusion::vector<Tree::Expr, Tree::Expr>(),
        SkipGrammar<Iterator>
      >
        pair
      ;
    };

    extern template class TupleGrammar<string_type::const_iterator>;
  }
}

#endif // TUPLE_PARSER_HPP_INCLUDED
