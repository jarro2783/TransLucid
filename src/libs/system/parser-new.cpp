/* The parser.
   Copyright (C) 2009, 2010, 2011 Jarryd Beck and John Plaice

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

#include <tl/parser-new.hpp>

namespace TransLucid
{

namespace Parser
{

Parser::Parser(System& system, Context& context)
: m_context(context), m_idents(system.lookupIdentifiers())
{
}

TreeNew::Expr
Parser::parse_expr(LexerIterator& begin, const LexerIterator& end)
{
  return parse_where(begin, end);
}

TreeNew::Expr
Parser::parse_where(LexerIterator& begin, const LexerIterator& end)
{
}

}
      
}
