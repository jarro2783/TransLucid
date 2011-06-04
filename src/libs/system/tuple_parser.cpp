/* Instantiation of TupleGrammar.
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

#include <tl/tuple_parser.hpp>

namespace TransLucid
{
  namespace Parser
  {
    namespace ph = boost::phoenix;

    struct make_tuple_imp
    {
      /**
       * The return type of make_tuple_imp.
       */
      template <typename Arg>
      struct result
      {
        /**
         * The actual type of the return type.
         */
        typedef std::vector<std::pair<Tree::Expr, Tree::Expr>> type;
      };

      /**
       * Make a tuple. Creates a tuple from the fusion vector that spirit
       * qi gives us of the parsed tuple.
       */
      template <typename Arg>
      typename result<Arg>::type
      operator()(const Arg& a) const
      {
        using boost::fusion::at_c;

        typename result<Arg>::type t;
        for (auto& v : a)
        {
          t.push_back(std::make_pair(at_c<0>(v), at_c<1>(v)));
        }

        return t;
      }
    };

    /**
     * Make a tuple.
     * The phoenix function to make a tuple.
     */
    ph::function<make_tuple_imp> make_tuple;

    template class TupleGrammar<iterator_t>;
    
    template <typename Iterator>
    template <typename TokenDef>
    TupleGrammar<Iterator>::TupleGrammar(TokenDef& tok)
    : TupleGrammar::base_type(context_perturb)
    {
      using namespace qi::labels;
      using namespace boost::phoenix;
      namespace phoenix = boost::phoenix;

      tuple_inside = pair[push_back(_val, _1)] % tok.comma_;

      pair =
        (
           expr
         >  tok.maps_
         > expr
        )
        [
          _val = construct<boost::fusion::vector<Tree::Expr, Tree::Expr>>
                 (_1, _3)
        ]
      ;

      context_perturb =
         (
            tok.lbracket_
          > tuple_inside
          > tok.rbracket_
         )
         [
           _val = construct<Tree::TupleExpr>(make_tuple(_2))
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

    TupleGrammar<iterator_t>*
    create_tuple_grammar(Lexer::tl_lexer& l)
    {
      return new TupleGrammar<iterator_t>(l);
    }
  }
}
