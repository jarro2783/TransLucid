/* Lexer tokens.
   Copyright (C) 2011 Jarryd Beck

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

#ifndef TL_LEXER_TOKENS_HPP_INCLUDED
#define TL_LEXER_TOKENS_HPP_INCLUDED

namespace TransLucid
{
  namespace Parser
  {
    //if this is modified, then modify token_name in libs/system/parser.cpp
    enum TokenType
    {
      TOKEN_EOF = 0, //end of file is 0 according to lexertl
      TOKEN_ASSIGNTO, //:=
      TOKEN_AT,
      TOKEN_BANG,
      TOKEN_CONSTANT,
      TOKEN_CONSTANT_RAW, //these two are not matched, use TOKEN_CONSTANT
      TOKEN_CONSTANT_INTERPRETED,
      TOKEN_COLON,
      TOKEN_COMMA,
      TOKEN_DARROW,
      TOKEN_DBLPERCENT,   //10
      TOKEN_DBLSEMI,
      TOKEN_DBLSLASH,
      TOKEN_DECLID,
      TOKEN_DIM_IDENTIFIER,
      TOKEN_DOT,
      TOKEN_ELSE,
      TOKEN_ELSIF,
      TOKEN_END,
      TOKEN_EQUALS,
      TOKEN_FALSE,        //20
      TOKEN_FI,
      TOKEN_HASH,
      TOKEN_IF,
      TOKEN_ID,
      TOKEN_INTEGER,
      TOKEN_LARROW,
      TOKEN_LBRACE,
      TOKEN_LPAREN,
      TOKEN_LSQUARE,
      TOKEN_NOW,          //30
      TOKEN_OPERATOR, //don't match this, it's converted to the right op
      TOKEN_PIPE,
      TOKEN_RARROW,
      TOKEN_RBRACE,
      TOKEN_RPAREN,
      TOKEN_RSQUARE,
      TOKEN_SLASH,
      TOKEN_SLASH_UNDERSCORE,
      TOKEN_THEN,
      TOKEN_TRUE,
      TOKEN_UARROW,       //40
      TOKEN_UCHAR,
      TOKEN_UNARY,
      TOKEN_WHERE,

      TOKEN_PREFIX_OP,
      TOKEN_BINARY_OP,
      TOKEN_POSTFIX_OP,

      TOKEN_LAST //nothing, one past the last
    };

    enum {
      SYMBOL_BASE_APP = TOKEN_DOT,
      SYMBOL_VALUE_APP = TOKEN_BANG
    };
  }
}

#endif
