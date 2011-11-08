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
// every function called shall have the iterator at the place to start

#include <tl/charset.hpp>
#include <tl/fixed_indexes.hpp>
#include <tl/output.hpp>
#include <tl/parser-new.hpp>
#include <tl/types/string.hpp>
#include <tl/types_util.hpp>

#include <sstream>

#define XSTRING(x) STRING(x)
#define STRING(x) #x

namespace TransLucid
{

namespace Parser
{

u32string
build_position_message(const LexerIterator& iter)
{
  const Position& pos = iter.getPos();
  std::ostringstream os;
  os << "At " << pos.file << ":" << pos.line << ":" << pos.character
     << ": ";
  return utf8_to_utf32(os.str());
}

ExpectedToken::ExpectedToken(const LexerIterator& pos, 
  size_t token, const u32string& text)
: ParseError(utf32_to_utf8(build_position_message(pos) + 
             U"expected `" + text + U"`"))
, m_token(token)
{
}

ExpectedExpr::ExpectedExpr(const LexerIterator& pos, const u32string& text)
: ParseError(utf32_to_utf8(build_position_message(pos) + U"expected " + text))
{
}

Parser::Parser(System& system)
: m_idents(system.lookupIdentifiers()), m_context(system.getDefaultContext())
{
}

/**
 * Finds a binary operator from a string.
 *
 * Tries to match the string @a s with an existing binary operator.
 * If it doesn't exist, then it uses the default error operator and
 * flags the parser that there was a problem.
 * @param s The text of the operator.
 * @param h The parser header.
 * @return The binary operator instance.
 * @todo Implement the error flagging.
 */
inline Tree::BinaryOperator
find_binary_operator
(
  const u32string& symbol, 
  const System::IdentifierLookup& idents,
  dimension_index symbolDim,
  Context& k
)
{
  //lookup ATL_SYMBOL, ASSOC, PREC

  //std::cerr << "looking up symbol " << symbol << std::endl;

  ContextPerturber p(k, {{symbolDim, Types::String::create(symbol)}});
  WS* atlWS = idents.lookup(U"ATL_SYMBOL");
  Constant atl = (*atlWS)(k);

  WS* assocWS = idents.lookup(U"ASSOC");
  Constant assoc = (*assocWS)(k);

  WS* precWS = idents.lookup(U"PREC");
  Constant prec = (*precWS)(k);

  const u32string& assocName = get_constant_pointer<u32string>(assoc);

  Tree::InfixAssoc ia = Tree::ASSOC_LEFT;
  if (assocName == U"LEFT")
  {
    ia = Tree::ASSOC_LEFT;
  }
  else if (assocName == U"RIGHT")
  {
    ia = Tree::ASSOC_RIGHT;
  }
  else if (assocName == U"NON")
  {
    ia = Tree::ASSOC_NON;
  }

  #if 0
  std::cerr << "retrieved op" << std::endl
            << "  symbol: " << symbol << std::endl
            << "  op    : " << get_constant_pointer<u32string>(atl)
            << std::endl
            << "  assoc : " << assocName << std::endl
            << "  prec  : " << get_constant_pointer<mpz_class>(prec)
            << std::endl;
  #endif

  return Tree::BinaryOperator
  {
    ia,
    get_constant_pointer<u32string>(atl),
    symbol,
    get_constant_pointer<mpz_class>(prec)
  };
}

/**
 * Construct a constant with a type.
 * When the type is ustring or uchar, it constructs the actual ustring
 * or uchar. Otherwise it constructs a Tree::LiteralExpr.
 */
inline Tree::Expr
construct_typed_constant(const LexerIterator& token)
{
  auto& val = get<std::pair<u32string, u32string>>(token->getValue());

  const std::u32string& type = val.first;
  const std::u32string& value = val.second;
  if (type == U"ustring") {
    if (!validate_ustring(value)) {
      //TODO error handling
      //throw ParseError(U"Invalid character in ustring");
    }
    return value;
  } else if (type == U"uchar") {
    char32_t v = value[0];
    if (!validate_uchar(v)) {
      //TODO error handling
      //throw ParseError(U"Invalid character");
    }
    return v;
  } else {
    return Tree::LiteralExpr(type, value);
  }
}

bool
Parser::parse_expr(LexerIterator& begin, const LexerIterator& end,
  Tree::Expr& result)
{
  return parse_where(begin, end, result);
}

bool
Parser::parse_where(LexerIterator& begin, const LexerIterator& end,
  Tree::Expr& result)
{
  LexerIterator current = begin;
  //binary expression
  Tree::Expr binary_expr;
  bool success = parse_binary_op(current, end, binary_expr);

  if (success)
  {
    Token w = *current;

    if (w == TOKEN_WHERE)
    {
      //TODO parse all of where clause
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
  Tree::Expr& result)
{
  LexerIterator current = begin;
  Tree::Expr app;
  bool success = parse_app_expr(current, end, app);
  
  if (success)
  {
    Token t = *current;
    while (t == TOKEN_BINARY_OP)
    {
      //TODO build binary op
      Tree::Expr rhs;
      ++current;
      expect(current, end, rhs, U"app_expr", &Parser::parse_app_expr);

      app = Tree::insert_binary_operator
      (
        find_binary_operator
        (
          get<u32string>(t.getValue()),
          m_idents,
          DIM_SYMBOL,
          m_context
        ),
        app, 
        rhs
      );

      t = *current;
    }
    begin = current;
    result = app;
  }

  return success;
}

bool
Parser::parse_app_expr(LexerIterator& begin, const LexerIterator& end,
  Tree::Expr& result)
{
  LexerIterator current = begin;

  Tree::Expr lhs;
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
        Tree::Expr rhs;
        bool parsedExpr = parse_prefix_expr(current, end, rhs);

        if (!parsedExpr)
        {
          parsingApp = false;
        }
        else
        {
          //named application
          lhs = Tree::PhiAppExpr(lhs, rhs);
        }
      }
    }
    result = lhs;
    begin = current;
  }

  return success;
}

bool
Parser::parse_token_app(LexerIterator& begin, const LexerIterator& end,
  Tree::Expr& result)
{
  LexerIterator current = begin;
  bool success = true;
  switch (current->getType())
  {
    case TOKEN_AT:
    {
      Tree::Expr rhs;
      ++current;
      expect(current, end, rhs, U"prefix_expr", &Parser::parse_prefix_expr);
      //build at expr
      result = Tree::AtExpr(result, rhs);
    }
    break;

    case TOKEN_DOT:
    {
      Tree::Expr rhs;
      ++current;
      expect(current, end, rhs, U"prefix_expr", &Parser::parse_prefix_expr);
      //value application
      result = Tree::LambdaAppExpr(result, rhs);
    }
    break;

    case TOKEN_BANG:
    {
      Tree::Expr rhs;
      ++current;

      if (current->getType() == TOKEN_LPAREN)
      {
        ++current;
        //parse expression list
        LexerIterator listIter = current;
        std::vector<Tree::Expr> exprList;
        Tree::Expr element;

        bool makingList = true;
        while (makingList)
        {
          expect(listIter, end, element, U"expr", &Parser::parse_expr);
          exprList.push_back(std::move(element));

          if (listIter->getType() != TOKEN_COMMA) { makingList = false; }
        }

        expect(current, end, U")", TOKEN_RPAREN);

        //build a host function with a list of arguments
        result = Tree::BangAppExpr(result, exprList);
      }
      else
      {
        expect(current, end, rhs, U"prefix_expr", &Parser::parse_prefix_expr);
        //host function application with one argument
        result = Tree::BangAppExpr(result, rhs);
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
  Tree::Expr& result)
{
  if (*begin == TOKEN_PREFIX_OP)
  {
    LexerIterator current = begin;
    ++current;
    Tree::Expr rhs;
    expect(current, end, rhs, U"prefix_expr", &Parser::parse_prefix_expr);

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
  Tree::Expr& result)
{
  LexerIterator current = begin;
  Tree::Expr lhs;

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
  Tree::Expr& result)
{
  if (*begin == TOKEN_IF)
  {
    LexerIterator current = begin;
    ++current;

    Tree::Expr cond;
    expect(current, end, cond, U"expr", &Parser::parse_expr);

    expect(current, end, U"then", TOKEN_THEN);

    Tree::Expr action;
    expect(current, end, action, U"expr", &Parser::parse_expr);
    
    //then some elsifs
    std::vector<std::pair<Tree::Expr, Tree::Expr>> elsifs;
    while (*current == TOKEN_ELSIF)
    {
      ++current;

      Tree::Expr econd;
      expect(current, end, econd, U"expr", &Parser::parse_expr); 

      expect(current, end, U"then", TOKEN_THEN);

      Tree::Expr ethen;
      expect(current, end, ethen, U"expr", &Parser::parse_expr); 

      elsifs.push_back(std::make_pair(econd, ethen));
    }

    expect(current, end, U"else", TOKEN_ELSE);

    Tree::Expr elseexpr;
    expect(current, end, elseexpr, U"expr", &Parser::parse_expr);

    expect(current, end, U"fi", TOKEN_FI);

    result = Tree::IfExpr(cond, action, elsifs, elseexpr);

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
  Tree::Expr& result)
{
  bool success = true;
  switch(begin->getType())
  {
    case TOKEN_INTEGER:
    result = get<mpz_class>(begin->getValue());
    ++begin;
    break;

    case TOKEN_TRUE:
    result = true;
    ++begin;
    break;

    case TOKEN_FALSE:
    result = false;
    ++begin;
    break;

    case TOKEN_ID:
    result = Tree::IdentExpr(get<u32string>(begin->getValue()));
    ++begin;
    break;

    case TOKEN_CONSTANT_RAW:
    case TOKEN_CONSTANT_INTERPRETED:
    result = construct_typed_constant(begin);
    ++begin;
    break;

    case TOKEN_LSQUARE:
    //TODO a tuple
    break;

    case TOKEN_LPAREN:
    {
      LexerIterator current = begin;
      ++current;
      Tree::Expr e;
      expect(current, end, e, U"expr", &Parser::parse_expr);
      expect(current, end, U")", TOKEN_RPAREN);
      result = e;
      begin = current;
    }
    break;

    case TOKEN_SLASH:
    {
      LexerIterator current = begin;
      ++current;
      parse_function(current, end, result, TOKEN_SLASH);
      begin = current;
    }
    break;

    case TOKEN_DBLSLASH:
    {
      LexerIterator current = begin;
      ++current;
      parse_function(current, end, result, TOKEN_SLASH);
      begin = current;
    }
    break;

    case TOKEN_HASH:
    result = Tree::HashSymbol();
    ++begin;
    break;

    default:
    success = false;
  }

  return success;
}

void
Parser::parse_function(LexerIterator& begin, const LexerIterator& end,
  Tree::Expr& result,
  size_t type)
{
  LexerIterator current = begin;

  //parse the dimension capture list
  std::vector<Tree::Expr> captures;
  if (*current == TOKEN_LBRACE)
  {
    bool parsingList = true;

    while (parsingList)
    {
      Tree::Expr e;
      expect(current, end, e, U"expr", &Parser::parse_expr);

      captures.push_back(std::move(e));

      if (*current != TOKEN_COMMA) { parsingList = false; }
    }

    expect(current, end, U"}", TOKEN_RBRACE);
  }

  expect(current, end, U"identifier", TOKEN_ID);
  u32string name = get<u32string>(current->getValue());

  expect(current, end, U"->", TOKEN_RARROW);

  Tree::Expr rhs;
  expect(current, end, rhs, U"expr", &Parser::parse_expr);

  if (type == TOKEN_SLASH)
  {
    result = Tree::LambdaExpr(
      std::move(captures), std::move(name), std::move(rhs));
  }
  else if (type == TOKEN_DBLSLASH)
  {
    result = Tree::PhiExpr(
      std::move(captures), std::move(name), std::move(rhs));
  }
  else
  {
    throw "internal compiler error at: " __FILE__ ":" XSTRING(__LINE__);
  }

  //once it has all worked we can move along the iterator
  begin = current;
}

void
Parser::expect(LexerIterator& begin, const LexerIterator& end, 
  const u32string& message,
  size_t token
)
{
  if (begin == end || *begin != token)
  {
    throw ExpectedToken(begin, token, message);
  }

  ++begin;
}

void
Parser::expect(LexerIterator& begin, const LexerIterator& end, 
  Tree::Expr& result, const std::u32string& message,
  bool (Parser::*parser)(LexerIterator&, const LexerIterator&, Tree::Expr&)
)
{
  LexerIterator current = begin;
  bool success = (this->*parser)(current, end, result);

  if (!success)
  {
    throw ExpectedExpr(begin, message);
  }

  begin = current;
}

Token
Parser::nextToken(LexerIterator& begin)
{
  ++begin;
  return *begin;
}

}
      
}
