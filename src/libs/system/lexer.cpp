/* TransLucid lexer definition.
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

#include <tl/ast.hpp>
#include <tl/lexer.hpp>

namespace TransLucid
{

namespace Lexer
{

template <typename Lexer>
lex_tl_tokens<Lexer>::lex_tl_tokens(Parser::Errors& errors)
: if_(L"if")
, fi_(L"fi")
, where_(L"where")
, then_(L"then")
, elsif_(L"elsif")
, else_(L"else")
, true_(L"true")
, false_(L"false")
, spaces(L"([ \\n\\t])|(\\/\\/[^\\n]*)")
, m_errors(errors)
{
  using boost::phoenix::ref;
  using lex::_val;
  using lex::_start;
  using lex::_end;
  using lex::_state;
  using lex::_pass;
  namespace ph = boost::phoenix;

  this->self.add_pattern(L"IDENT", L"[A-Za-z][_A-Za-z0-9]*");

  this->self.add_pattern(L"DIGIT",     L"[0-9]");
  this->self.add_pattern(L"ADIGIT",    L"[0-9A-Za-z]");

  this->self.add_pattern(L"intDEC",    L"[1-9]{DIGIT}*");
  this->self.add_pattern(L"intNONDEC", L"0[2-9A-Za-z]{ADIGIT}+");
  this->self.add_pattern(L"intUNARY",  L"011+");

  this->self.add_pattern(L"floatDEC", 
    L"{intDEC}\\.{DIGIT}*(\\^~?{ADIGIT}+)?(#{DIGIT}+)?");
  this->self.add_pattern(L"floatNONDEC",
    L"{intNONDEC}\\.{ADIGIT}*(\\^~?{ADIGIT}+)?(#{ADIGIT}+)?");

  this->self.add_pattern(L"ratDEC",    L"{intDEC}_{intDEC}");
  this->self.add_pattern(L"ratNONDEC", L"{intNONDEC}_{ADIGIT}+");

  this->self.add_pattern(L"stringINTERPRET", 
    L"\\\"([^\\\"\\\\]|\\\\.)*\\\"");
  this->self.add_pattern(L"stringRAW", L"`[^`]*`");

  identifier_        = L"{IDENT}";
  constantRAW_       = L"{IDENT}?{stringRAW}";
  constantINTERPRET_ = L"{IDENT}?{stringINTERPRET}";
  integer_           = L"0|(~?({intDEC}|{intNONDEC}|{intUNARY}))";
  character_         = L"'([^'\\\\]|\\\\.)+'";

  dblslash_   = L"\\\\\\\\";
  range_      = L"\\.\\.";
  arrow_      = L"->";
  dblsemi_    = L";;";
  dbldollar_  = L"\\$\\$";
  dblpercent_ = L"%%";

  real_     = L"(0\\.0)|~?({floatDEC}|{floatNONDEC})";
  rational_ = L"(0_1)|(~?)({ratDEC}|{ratNONDEC})";

  library_ =      L"library";
  dimension_ =    L"dimension";
  infix_binary_ = L"infix[lrnpm]";
  unary_ =        L"(prefix)|(postfix)";

  any_ = L".+?";

  this->self =
    spaces[lex::_pass = lex::pass_flags::pass_ignore]
  //multi character symbols
  | if_
  | fi_
  | where_
  | then_
  | elsif_
  | else_
  | true_
  | false_
  | range_
  | dblslash_
  | arrow_
  | dblsemi_
  | dbldollar_
  | dblpercent_

  //constants
  | constantRAW_       [detail::build_constant()]
  | constantINTERPRET_ [detail::build_constant()]
  | character_         [detail::build_character()]

  //numbers
  | integer_ [detail::build_integer(m_errors)]
  | real_    [detail::build_real()]
  | rational_[detail::build_rational()]

  //header items
  | library_
  | dimension_
  | infix_binary_
  | unary_

  //single character symbols
  | L':'
  | L'['
  | L']'
  | L'.'
  | L'='
  | L'&'
  | L'#'
  | L'@'
  | L'\\'
  | L'('
  | L')'
  | L'|'
  | L','

  | identifier_
  ;

  //anything else not matched
  this->self.add(any_)
  ;
}

template lex_tl_tokens<lexer_type>::lex_tl_tokens(Parser::Errors&);

} //namespace Lexer

} //namespace TransLucid
