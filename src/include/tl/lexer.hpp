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

/**
 * @file lexer.hpp
 * The lexer implementation.
 * The variables in the lexer are named such that any variable with an
 * underscore at the end of the name is a token.
 */

#ifndef TL_LEXER_HPP_INCLUDED
#define TL_LEXER_HPP_INCLUDED

#include <boost/spirit/include/lex_lexertl.hpp>
#include <boost/spirit/home/phoenix/operator.hpp>
#include <boost/spirit/home/phoenix/bind.hpp>
#include <boost/spirit/home/phoenix/statement/sequence.hpp>
#include <boost/spirit/home/phoenix/object/construct.hpp>

#include <tl/charset.hpp>
#include <tl/utility.hpp>
#include <tl/lexer_util.hpp>
#include <tl/detail/lexer_detail.hpp>
#include <tl/parser_iterator.hpp>

namespace TransLucid
{
  namespace Lexer
  {
    namespace lex = boost::spirit::lex;
    typedef wchar_t lex_char_type;

    // iterator type used to expose the underlying input stream
    typedef std::basic_string<lex_char_type> wstring;
    //typedef wstring::const_iterator base_iterator_type;
    typedef Parser::U32Iterator base_iterator_t;

    // This is the token type to return from the lexer iterator
    typedef lex::lexertl::token<
      base_iterator_t, 
      boost::mpl::vector
      <
        value_wrapper<mpz_class>, 
        value_wrapper<mpq_class>,
        value_wrapper<mpf_class>,
        u32string, 
        std::pair<u32string, u32string>,
        char32_t
      > 
    > token_type;

    // This is the lexer type to use to tokenize the input.
    // We use the lexertl based lexer engine.
    typedef lex::lexertl::actor_lexer<token_type> lexer_type;

    //for unnamed tokens
    typedef lex::token_def<lex::unused_type, lex_char_type> token_def_default;

    template <typename Lexer>
    struct lex_tl_tokens : lex::lexer<Lexer>
    {
      lex_tl_tokens()
      : if_(L"if")
      , fi_(L"fi")
      , where_(L"where")
      , then_(L"then")
      , elsif_(L"elsif")
      , else_(L"else")
      , true_(L"true")
      , false_(L"false")
      , spaces(L"[ \\n\\t]")
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

        dblslash_ = L"\\\\\\\\";
        range_    = L"\\.\\.";
        arrow_    = L"->";
        dblsemi_  = L";;";

        real_     = L"(0\\.0)|~?({floatDEC}|{floatNONDEC})";
        rational_ = L"(0_1)|(~?)({ratDEC}|{ratNONDEC})";

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
        | identifier_
        | range_
        | dblslash_
        | arrow_
        | dblsemi_

        //constants
        | constantRAW_       [detail::build_constant()]
        | constantINTERPRET_ [detail::build_constant()]
        | character_         [detail::build_character()]

        //numbers
        | integer_ [detail::build_integer()]
        | real_    [detail::build_real()]
        | rational_[detail::build_rational()]

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
        ;

        this->self.add(L".");
      }

      lex::token_def<lex::unused_type, lex_char_type> 
        //keywords
        if_, fi_, where_, then_, elsif_, else_, true_, false_
        //symbols
      , arrow_, dblsemi_, dblslash_, range_
        //white space
      , spaces
      ;

      lex::token_def<u32string, lex_char_type>
        identifier_
      ;

      lex::token_def<value_wrapper<mpz_class>, lex_char_type> 
        integer_
      ;

      lex::token_def<value_wrapper<mpf_class>, lex_char_type>
        real_
      ;

      lex::token_def<value_wrapper<mpq_class>, lex_char_type>
        rational_
      ;

      lex::token_def<std::pair<u32string, u32string>, lex_char_type>
      //these are the two that the parser should match
        constantINTERPRET_
      , constantRAW_
      ;

      lex::token_def<char32_t, lex_char_type> 
        character_
      ;

      private:
      std::wstring m_constant_type;
      std::wstring m_constant_value;
    };

    //the lexer class
    typedef lex_tl_tokens<lexer_type> tl_lexer;

    // This is the iterator type exposed by the lexer 
    typedef tl_lexer::iterator_type iterator_t;
  }
}

#endif
