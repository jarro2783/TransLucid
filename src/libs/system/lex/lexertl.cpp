/* Lexer using lexertl.
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

#include "lexertl/generator.hpp"
#include "lexertl/generate_cpp.hpp"
#include "lexertl/rules.hpp"
#include "lexertl/state_machine.hpp"
#include "tl/lexer_tokens.hpp"

namespace TransLucid
{
  namespace Parser
  {
    class GenerateLexer
    {
      typedef lexertl::basic_rules<char32_t> lexrules;
      lexrules m_rules;

      typedef lexertl::basic_state_machine<char32_t> lexstate;
      lexstate m_state_machine;

      typedef lexertl::basic_generator<lexrules, lexstate> generator;

      public:
      GenerateLexer()
      {
        //macros
        //m_rules.add_macro(U"IDENT", U"[A-Za-z][_A-Za-z0-9]*");
        m_rules.add_macro(U"IDENT", U"[\\p{L}][_\\p{L}0-9]*");

        m_rules.add_macro(U"DIGIT",     U"[0-9]");
        m_rules.add_macro(U"ADIGIT",    U"[0-9A-Za-z]");

        m_rules.add_macro(U"intDEC",    U"[1-9]{DIGIT}*");
        m_rules.add_macro(U"intNONDEC", U"0[2-9A-Za-z]{ADIGIT}+");
        m_rules.add_macro(U"intUNARY",  U"011+");

        m_rules.add_macro(U"floatDEC", 
          U"{intDEC}\\.{DIGIT}*(\\^~?{ADIGIT}+)?(#{DIGIT}+)?");
        m_rules.add_macro(U"floatNONDEC",
          U"{intNONDEC}\\.{ADIGIT}*(\\^~?{ADIGIT}+)?(#{ADIGIT}+)?");

        m_rules.add_macro(U"ratDEC",    U"{intDEC}_{intDEC}");
        m_rules.add_macro(U"ratNONDEC", U"{intNONDEC}_{ADIGIT}+");

        m_rules.add_macro(U"stringINTERPRET", 
          U"\\\"([^\\\"\\\\]|\\\\.)*\\\"");
        m_rules.add_macro(U"stringRAW", U"`[^`]*`");
        
        //the rules
        m_rules.add(U"else", TOKEN_ELSE);
        m_rules.add(U"elsif", TOKEN_ELSIF);
        m_rules.add(U"end", TOKEN_END);
        m_rules.add(U"false", TOKEN_FALSE);
        m_rules.add(U"fi", TOKEN_FI);
        m_rules.add(U"if", TOKEN_IF);
        m_rules.add(U"then", TOKEN_THEN);
        m_rules.add(U"true", TOKEN_TRUE);
        m_rules.add(U"where(_{IDENT})?", TOKEN_WHERE);

        m_rules.add(U"{IDENT}?{stringRAW}", TOKEN_CONSTANT_RAW);
        m_rules.add(U"{IDENT}?{stringINTERPRET}", TOKEN_CONSTANT_INTERPRETED);
        m_rules.add(U"'([^'\\\\]|\\\\.)+'", TOKEN_UCHAR);

        m_rules.add(U"@", TOKEN_AT);
        m_rules.add(U"!", TOKEN_BANG);
        m_rules.add(U":", TOKEN_COLON);
        m_rules.add(U",", TOKEN_COMMA);
        m_rules.add(U"%%", TOKEN_DBLPERCENT);
        m_rules.add(U";;", TOKEN_DBLSEMI);
        m_rules.add(UR"(\\\\)", TOKEN_DBLSLASH);
        m_rules.add(U"\\.", TOKEN_DOT);
        m_rules.add(U"=", TOKEN_EQUALS);
        m_rules.add(U"#", TOKEN_HASH);
        m_rules.add(U"<-", TOKEN_LARROW);
        m_rules.add(U"\\{", TOKEN_LBRACE);
        m_rules.add(U"\\(", TOKEN_LPAREN);
        m_rules.add(U"\\[", TOKEN_LSQUARE);
        m_rules.add(U"\\|", TOKEN_PIPE);
        m_rules.add(UR"(\.\.)", TOKEN_RANGE);
        m_rules.add(U"->", TOKEN_RARROW);
        m_rules.add(U"\\}", TOKEN_RBRACE);
        m_rules.add(U"\\)", TOKEN_RPAREN);
        m_rules.add(U"\\]", TOKEN_RSQUARE);
        m_rules.add(UR"(\\)", TOKEN_SLASH);

        m_rules.add(U"{IDENT}", TOKEN_ID);
        m_rules.add(U"0|(~?({intDEC}|{intNONDEC}|{intUNARY}))", TOKEN_INTEGER);

        m_rules.add(U"([ \\r\\n\\t])|(\\/\\/([^\\n]*)\\n)", 
          m_state_machine.skip());
        
        m_rules.add(UR"**([\+!\$%\^&\|\*\-_\?\/<>=\p{S}]+)**", TOKEN_OPERATOR);
        //, spaces(U"[ \\n\\t]")
        //, binary_op_(U".", OpTokens::TOK_BINARY_OP)
        //, prefix_op_(U".", OpTokens::TOK_PREFIX_OP)
        //, postfix_op_(U".", OpTokens::TOK_POSTFIX_OP)
        generator::build(m_rules, m_state_machine);
        m_state_machine.minimise();

        lexertl::table_based_cpp::generate_cpp("translucid_lex", 
          m_state_machine, false, std::cout);
      }
    };
  }
}

int main()
{
  TransLucid::Parser::GenerateLexer lex;
  return 0;
}
