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

#include <tl/expr_parser.hpp>

namespace TransLucid
{

   namespace Parser
   {
      template class ExprGrammar<string_type::const_iterator>;
   }

}

#if 0
#include <tl/interpreter.hpp>

namespace TransLucid
{

void
Interpreter::initExprParser()
{
  //m_exprGrammar =
  //  new Parser::ExprGrammar<std::u32string::const_iterator>(m_parseInfo);
  //m_parsers.expr_parser.push(*m_exprGrammar);
}

void
Interpreter::cleanupExprParser()
{
   //delete m_exprGrammar;
}

}

#endif
