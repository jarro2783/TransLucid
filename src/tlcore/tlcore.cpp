/* Core TransLucid application implementation.
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

#include "tlcore.hpp"
#include <tl/expr_parser.hpp>
#include <tl/equation_parser.hpp>
#include <tl/tuple_parser.hpp>

namespace TransLucid
{

namespace TLCore
{

template <typename Iterator>
class Grammar : 
  public Parser::qi::grammar<Iterator, Parser::SkipGrammar<Iterator>>
{
  public:
  Grammar()
  : Grammar::base_type(r_program)
   ,m_expr(m_header)
  {
    m_tuple.set_expr(m_expr);
    m_expr.set_tuple(m_tuple);
    m_eqn.set_expr(m_expr);
    m_eqn.set_tuple(m_tuple);

    r_program = 
       r_header
    >> "%%"
    >> r_eqns
    >> "%%"
    >> r_exprs
    >> Parser::qi::eoi
    ;

    r_eqns = *(m_eqn >> ";;");

    r_exprs = *(m_expr >> ";;");
  }

  private:
  Parser::Header m_header;
  Parser::ExprGrammar<Iterator> m_expr;
  Parser::EquationGrammar<Iterator> m_eqn;
  Parser::TupleGrammar<Iterator> m_tuple;

  Parser::qi::rule<Iterator, Parser::SkipGrammar<Iterator>> 
    r_program,
    r_header,
    r_exprs,
    r_eqns
  ;
};

TLCore::TLCore()
: 
  m_verbose(false)
 ,m_reactive(false)
 ,m_is(&std::cin)
 ,m_os(&std::cout)
{
  m_grammar = new Grammar<Parser::string_type::const_iterator>;
}

void 
TLCore::run()
{
  std::u32string input = read_input();
}

std::u32string 
TLCore::read_input()
{
  std::u32string input;
  std::stringbuf sb;

  while (*m_is)
  {
    m_is->get(sb);
  }

  input = utf8_to_utf32(sb.str());
  return input;
}

} //namespace TLCore

} //namespace TransLucid
