/* Instantiation of EquationGrammar.
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

/**
 * @file equation_parser.cpp 
 * The equations parser template instantiation.
 */

#include <tl/equation_parser.hpp>
#include <tl/output.hpp>

namespace TransLucid
{
  namespace Parser
  {
    template <typename Iterator>
    template <typename TokenDef>
    EquationGrammar<Iterator>::EquationGrammar(TokenDef& tok)
    : EquationGrammar::base_type(equation)
    {
      using namespace qi::labels;

      equation =
        (
            tok.identifier_
         >> guard
         >> boolean
         >> declaration
         >> expr
        )
        [
          _val = construct<std::pair<Equation, DeclType>>
            (
              construct<Equation>(_1, _2, _3, _5),
              _4
            )
        ]
      ;

      guard =
        ( context_guard(true)[_val = _1])
      | qi::eps [_val = construct<Tree::nil>()]
      ;

      boolean =
        ( tok.pipe_ >> expr[_val = _1])
      | qi::eps [_val = construct<Tree::nil>()]
      ;

      declaration =
        tok.def_[_val = DECL_DEF]
      | tok.assign_[_val = DECL_ASSIGN]
      ;

      BOOST_SPIRIT_DEBUG_NODE(boolean);
      BOOST_SPIRIT_DEBUG_NODE(guard);
      BOOST_SPIRIT_DEBUG_NODE(equation);
      BOOST_SPIRIT_DEBUG_NODE(expr);
      BOOST_SPIRIT_DEBUG_NODE(context_guard);
    }

    template class EquationGrammar<iterator_t>;

    //template EquationGrammar<iterator_t>::
    //  EquationGrammar(Lexer::tl_lexer&);

    EquationGrammar<iterator_t>*
    create_equation_grammar(Lexer::tl_lexer& l)
    {
      return new EquationGrammar<iterator_t>(l);
    }
  }
}
