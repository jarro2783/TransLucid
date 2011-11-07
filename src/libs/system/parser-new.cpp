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
      expect(current, end, U"end", TOKEN_END);
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
  LexerIterator current = begin;

  TreeNew::Expr lhs;
  bool success = parse_prefix_expr(current, end, lhs);

  if (success)
  {
    bool parsingApp = true;
    Token next = *current;

    while (parsingApp)
    {
      //this one will modify the lhs and make a binary app expr from it
      if (!parse_token_app(current, end, lhs))
      {
        TreeNew::Expr rhs;
        bool parsedExpr = parse_prefix_expr(current, end, rhs);

        if (!parsedExpr)
        {
          parsingApp = false;
        }
        else
        {
          //named application
          lhs = TreeNew::PhiAppExpr(lhs, rhs);
        }
      }
    }
    begin = current;
  }

  return success;
}

bool
Parser::parse_token_app(LexerIterator& begin, const LexerIterator& end,
  TreeNew::Expr& result)
{
  LexerIterator current = begin;
  bool success = true;
  switch (current->getType())
  {
    case TOKEN_AT:
    {
      TreeNew::Expr rhs;
      ++current;
      expect(current, end, rhs, U"expr", &Parser::parse_prefix_expr);
      //build at expr
      result = TreeNew::AtExpr(result, rhs);
    }
    break;

    case TOKEN_DOT:
    {
      TreeNew::Expr rhs;
      ++current;
      expect(current, end, rhs, U"expr", &Parser::parse_prefix_expr);
      //value application
      result = TreeNew::LambdaAppExpr(result, rhs);
    }
    break;

    case TOKEN_BANG:
    {
      TreeNew::Expr rhs;
      ++current;

      if (current->getType() == TOKEN_LPAREN)
      {
        ++current;
        //parse expression list
        LexerIterator listIter = current;
        std::vector<TreeNew::Expr> exprList;
        TreeNew::Expr element;

        bool makingList = true;
        while (makingList)
        {
          expect(listIter, end, element, U"expr", &Parser::parse_expr);
          exprList.push_back(std::move(element));

          if (listIter->getType() != TOKEN_COMMA) { makingList = false; }
        }

        expect(current, end, U")", TOKEN_RPAREN);
        ++current;

        //build a host function with a list of arguments
        result = TreeNew::BangAppExpr(result, exprList);
      }
      else
      {
        expect(current, end, rhs, U"expr", &Parser::parse_prefix_expr);
        //host function application with one argument
        result = TreeNew::BangAppExpr(result, rhs);
      }
    }
    break;

    default:
    //fail
    success = false;
    break;
  }

  if (success) { begin = current; }
  return success;
}

bool
Parser::parse_prefix_expr(LexerIterator& begin, const LexerIterator& end,
  TreeNew::Expr& result)
{
  if (*begin == TOKEN_PREFIX_OP)
  {
    LexerIterator current = begin;
    TreeNew::Expr rhs;
    expect(current, end, rhs, U"expr", &Parser::parse_prefix_expr);

    //TODO build prefix op

    begin = current;
    return true;
  }
  else
  {
    return parse_postfix_expr(begin, end, result);
  }
}

bool
Parser::parse_postfix_expr(LexerIterator& begin, const LexerIterator& end,
  TreeNew::Expr& result)
{
  LexerIterator current = begin;
  TreeNew::Expr lhs;

  bool success = parse_if_expr(current, end, lhs);

  if (success)
  {
    if (*current == TOKEN_POSTFIX_OP)
    {
      //TODO build postfix op
    }
    else
    {
      result = lhs;
      begin = current;
    }
  }

  return success;
}

bool
Parser::parse_if_expr(LexerIterator& begin, const LexerIterator& end,
  TreeNew::Expr& result)
{
  LexerIterator current = begin;
  if (*current == TOKEN_IF)
  {
    ++current;

    TreeNew::Expr cond;
    expect(current, end, cond, U"expr", &Parser::parse_expr);

    expect(current, end, U"then", TOKEN_THEN);
    ++current;

    TreeNew::Expr action;
    expect(current, end, action, U"expr", &Parser::parse_expr);
    
    //then some elsifs
    std::vector<std::pair<TreeNew::Expr, TreeNew::Expr>> elsifs;
    while (*current == TOKEN_ELSIF)
    {
      ++current;

      TreeNew::Expr econd;
      expect(current, end, econd, U"expr", &Parser::parse_expr); 

      expect(current, end, U"then", TOKEN_THEN);
      ++current;

      TreeNew::Expr ethen;
      expect(current, end, ethen, U"expr", &Parser::parse_expr); 

      elsifs.push_back(std::make_pair(econd, ethen));
    }

    expect(current, end, U"else", TOKEN_ELSE);
    ++current;

    TreeNew::Expr elseexpr;
    expect(current, end, elseexpr, U"expr", &Parser::parse_expr);

    expect(current, end, U"fi", TOKEN_FI);

    result = TreeNew::IfExpr(cond, action, elsifs, elseexpr);

    begin = current;
    return true;
  }
  else
  {
    return parse_primary_expr(begin, end, result);
  }
}

bool
Parser::parse_primary_expr(LexerIterator& begin, const LexerIterator& end,
  TreeNew::Expr& result)
{
}


void
Parser::expect(LexerIterator& begin, const LexerIterator& end, 
  const u32string& message,
  size_t token
)
{
  if (begin == end || *begin != token)
  {
    throw ExpectedToken(token, message);
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
