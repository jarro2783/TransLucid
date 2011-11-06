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

// == The expression parser ==
// Every parse function must leave the iterator untouched if it didn't match,
// and one past the last token of what it matched if it did match

#include <tl/parser-new.hpp>
#include <tl/charset.hpp>

namespace TransLucid
{

namespace Parser
{

ExpectedToken::ExpectedToken(size_t token, const u32string& text)
: ParseError(utf32_to_utf8(U"expected `" + text + U"`"))
, m_token(token)
{
}

ExpectedExpr::ExpectedExpr(const u32string& text)
: ParseError(utf32_to_utf8(U"expected " + text))
{
}

Parser::Parser(System& system, Context& context)
: m_context(context), m_idents(system.lookupIdentifiers())
{
}

bool
Parser::parse_expr(LexerIterator& begin, const LexerIterator& end,
  TreeNew::Expr& result)
{
  return parse_where(begin, end, result);
}

bool
Parser::parse_where(LexerIterator& begin, const LexerIterator& end,
  TreeNew::Expr& result)
{
  LexerIterator current = begin;
  //binary expression
  TreeNew::Expr binary_expr;
  bool success = parse_binary_op(current, end, binary_expr);

  if (success)
  {
    Token w = *begin;

    if (w == TOKEN_WHERE)
    {
      ++current;
      expect(current, end, TOKEN_END, U"end");
    }
    else
    {
      result = binary_expr;
    }
    begin = current;
  }

  return success;
}

bool
Parser::parse_binary_op(LexerIterator& begin, const LexerIterator& end,
  TreeNew::Expr& result)
{
  LexerIterator current = begin;
  TreeNew::Expr app;
  bool success = parse_app_expr(current, end, app);
  

  if (success)
  {
    Token t = nextToken(current);
    while (t == TOKEN_BINARY_OP)
    {
      TreeNew::Expr rhs;
      expect(current, end, rhs, U"expr", &Parser::parse_app_expr);
      t = nextToken(current);
    }
    begin = current;
  }

  return success;
}

bool
Parser::parse_app_expr(LexerIterator& begin, const LexerIterator& end,
  TreeNew::Expr& result)
{
}

void
Parser::expect(LexerIterator& begin, const LexerIterator& end, size_t token,
  const u32string& message)
{
  if (begin == end || *begin != token)
  {
  }

  ++begin;
}

void
Parser::expect(LexerIterator& begin, const LexerIterator& end, 
  TreeNew::Expr& result, const std::u32string& message,
  bool (Parser::*parser)(LexerIterator&, const LexerIterator&, TreeNew::Expr&)
)
{
  LexerIterator current = begin;
  bool success = (this->*parser)(current, end, result);

  if (!success)
  {
    throw ExpectedExpr(message);
  }

  begin = current;
  ++begin;
}

Token
Parser::nextToken(LexerIterator& begin)
{
  ++begin;
  return *begin;
}

}
      
}
