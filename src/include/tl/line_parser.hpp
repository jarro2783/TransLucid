/* Parses lines.
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

#ifndef TL_LINE_PARSER_HPP_INCLUDED
#define TL_LINE_PARSER_HPP_INCLUDED

#include <boost/spirit/include/qi_grammar.hpp>
#include <boost/spirit/include/qi_rule.hpp>

#include <tl/equation_parser.hpp>

namespace TransLucid
{
  namespace Parser
  {
    inline Tree::UnaryOperator
    makeUnaryOp(Tree::UnaryType type, const Parser::UnopHeader& op)
    {
      return Tree::UnaryOperator(std::get<1>(op), std::get<0>(op), type);
    }

    inline Tree::BinaryOperator
    makeBinaryOp(Tree::InfixAssoc type, const Parser::BinopHeader& op)
    {
      return Tree::BinaryOperator(type, 
        std::get<1>(op), std::get<0>(op), std::get<2>(op));
    }

    template <typename Iterator>
    class LineGrammar : public qi::grammar<Iterator, Line>
    {
      public:
      template <typename TokenDef>
      LineGrammar
      (
        TokenDef& tok,
        EquationGrammar<Iterator>& eqn
      )
      : LineGrammar::base_type(r_line)
      , g_equation(eqn)
      , g_hstring(tok)
      , g_binop(tok)
      , g_unop(tok)
      {
        using namespace qi::labels;

        r_line = 
        (
          (tok.dimension_ > g_hstring[_val = construct<DimensionDecl>(_1)])
        | (tok.library_ > g_hstring[_val = construct<LibraryDecl>(_1)])
        | (tok.eqn_ > g_equation[_val = _1])
        | (tok.assignment_ > g_equation[_val = _1])
        | ((tok.infix_binary_ > g_binop)
          [
            ph::bind(&makeBinaryOp, _1, _2)
          ])
        | ((tok.unary_ > g_unop)
          [
            ph::bind(&makeUnaryOp, _1, _2)
          ])
        )

        > tok.dblsemi_
        ;
      }

      qi::rule<Iterator, Line> r_line;
      EquationGrammar<Iterator>& g_equation;
      HeaderStringGrammar<Iterator> g_hstring;
      HeaderBinopGrammar<Iterator> g_binop;
      HeaderUnopGrammar<Iterator> g_unop;
    };
  }
}

#endif
