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
    template class TupleGrammar<iterator_t>;
    
    template <typename Iterator>
    TupleGrammar<Iterator>::TupleGrammar()
    : TupleGrammar::base_type(context_perturb)
    {
      using namespace qi::labels;
      using namespace boost::phoenix;
      namespace phoenix = boost::phoenix;
      //expr = self.parsers.expr_parser.top();

      tuple_inside = pair[push_back(_val, _1)] % literal(',');

      pair %=
        (
           expr
         >  literal(':')
         > expr
        )
        //[
        //  _val = construct<boost::fusion::vector<Tree::Expr, Tree::Expr>>
        //         (_1, _2)
        //]
      ;

      context_perturb =
         (
            literal('[')
          > tuple_inside
          > literal(']')
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

    TupleGrammar<iterator_t>*
    create_tuple_grammar()
    {
      return new TupleGrammar<iterator_t>;
    }
  }
}
