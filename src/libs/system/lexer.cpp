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
#include <tl/lexer_util.hpp>
#include <tl/output.hpp>

namespace TransLucid
{

namespace Lexer
{

template <typename Lexer>
lex_tl_tokens<Lexer>::lex_tl_tokens(Parser::Errors& errors, System& system)
: else_(L"else")
, elsif_(L"elsif")
, end_(L"end")
, false_(L"false")
, fi_(L"fi")
, if_(L"if")
, then_(L"then")
, true_(L"true")
, spaces(L"([ \\r\\n\\t])|(\\/\\/([^\\n]*)\\n)")
//, spaces(L"[ \\n\\t]")
, binary_op_(L".", OpTokens::TOK_BINARY_OP)
, prefix_op_(L".", OpTokens::TOK_PREFIX_OP)
, postfix_op_(L".", OpTokens::TOK_POSTFIX_OP)
, m_context(nullptr)
, m_errors(errors)
, m_identifiers(system.lookupIdentifiers())
, m_symbolDim(system.getDimensionIndex(U"symbol"))
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

  where_             = L"where(_{IDENT})?";
  identifier_        = L"{IDENT}";
  constantRAW_       = L"{IDENT}?{stringRAW}";
  constantINTERPRET_ = L"{IDENT}?{stringINTERPRET}";
  integer_           = L"0|(~?({intDEC}|{intNONDEC}|{intUNARY}))";
  character_         = L"'([^'\\\\]|\\\\.)+'";

  bang_     = L'!';
  decl_     = L':';
  lbracket_ = L'[';
  rbracket_ = L']';
  dot_      = L'.'; 
  def_      = L'=';
  and_      = L'&';
  hash_     = L'#';
  at_       = L'@';
  slash_    = L'\\';
  lparen_   = L'(';
  rparen_   = L')';
  pipe_     = L'|';
  comma_    = L',';
  dollar_   = L"\\$";
  obrace_   = L"\\{";
  cbrace_   = L"\\}";

  bang_abstract_ = LR"(\\b)";
  maps_       = L"<-";
  dblslash_   = LR"(\\\\)";
  range_      = LR"(\.\.)";
  arrow_      = L"->";
  dblsemi_    = L";;";
  //dbldollar_  = L"\\$\\$\\n";
  dblpercent_ = L"%%";
  assign_     = L":=";

  //real_     = L"(0\\.0)|~?({floatDEC}|{floatNONDEC})";
  rational_ = L"(0_1)|(~?)({ratDEC}|{ratNONDEC})";

  library_      = L"library";
  dimension_    = L"dim";
  assignment_   = L"assign";
  var_          = L"var";
  infix_binary_ = L"infix[lrnpm]";
  unary_        = L"(prefix)|(postfix)";
  out_          = L"out";
  in_           = L"in";

  operator_ = LR"**([\+!\$%\^&|\*\-_\:\?/<>=]+)**";

  this->self =
    spaces[lex::_pass = lex::pass_flags::pass_ignore]
  //multi character symbols
  | arrow_
  | assign_
  | bang_
  | bang_abstract_
  | dblpercent_
  | dblsemi_
  | dblslash_
  | end_
  | else_
  | elsif_
  | false_
  | fi_
  | if_
  | maps_
  | range_ [detail::handle_range()]
  | then_
  | true_
  //| dbldollar_
  | where_ [detail::build_where()]

  //constants
  | constantRAW_       [detail::build_constant()]
  | constantINTERPRET_ [detail::build_constant()]
  | character_         [detail::build_character()]

  //numbers
  | integer_ [detail::build_integer(m_errors)]
  //| real_    [detail::build_real()]
  | rational_[detail::build_rational()]

  //type of line items
  | library_
  | dimension_
  | assignment_
  | var_
  | infix_binary_
  | unary_
  | out_
  | in_

  //single character symbols
  | decl_
  | lbracket_
  | rbracket_
  | dot_
  | def_
  | and_
  | hash_
  | at_
  | slash_
  | lparen_
  | rparen_
  | pipe_
  | comma_
  | dollar_
  
  | identifier_
  //the parser should not much operator_, instead it should match
  //binary_op_, prefix_op_ and postfix_op_
  | operator_ 
    [
      //detail::handle_operator<decltype(binary_op_)>(m_identifiers, binary_op_)
      detail::handle_operator
      (
        m_identifiers, 
        m_context, 
        m_symbolDim
      )
    ]
  ;

  this->self.add(binary_op_);
  this->self.add(prefix_op_);
  this->self.add(postfix_op_);

  //anything else not matched
  //this->self.add(operator_);
}

template lex_tl_tokens<lexer_type>::lex_tl_tokens(Parser::Errors&, System&);

} //namespace Lexer

} //namespace TransLucid
