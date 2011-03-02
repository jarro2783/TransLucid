/* TransLucid lexer definition.
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

#include <boost/spirit/include/lex_lexertl.hpp>
#include <boost/spirit/home/phoenix/operator.hpp>
#include <boost/spirit/home/phoenix/bind.hpp>
#include <boost/spirit/home/phoenix/statement/sequence.hpp>
#include <boost/spirit/home/phoenix/object/construct.hpp>

#include <tl/charset.hpp>
#include <tl/utility.hpp>
#include <tl/lexer_util.hpp>
#include <tl/detail/lexer.hpp>

namespace TransLucid
{
  namespace Parser
  {
    namespace lex = boost::spirit::lex;
    //for unnamed tokens
    typedef lex::token_def<lex::unused_type, wchar_t> token_def_default;

    template <typename Lexer>
    struct lex_tl_tokens : lex::lexer<Lexer>
    {
      lex_tl_tokens()
      : if_(L"if")
      , fi_(L"fi")
      , where_(L"where")
      , then_(L"then")
      , elsif_(L"elsif")
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

        this->self.add_pattern(L"DIGIT", L"[0-9]");
        this->self.add_pattern(L"ADIGIT", L"[0-9A-Za-z]");
        this->self.add_pattern(L"intDEC", L"[1-9]{DIGIT}*");
        this->self.add_pattern(L"intNONDEC", L"0[2-9A-Za-z]{ADIGIT}+");
        this->self.add_pattern(L"intUNARY", L"011+");
        this->self.add_pattern(L"floatDEC", 
          L"{intDEC}\\.{DIGIT}*(\\^~?{ADIGIT}+)?(#{DIGIT}+)?");
        this->self.add_pattern(L"floatNONDEC",
          L"{intNONDEC}\\.{ADIGIT}*(\\^~?{ADIGIT}+)?(#{ADIGIT}+)?");
        this->self.add_pattern(L"ratDEC", L"{intDEC}_{intDEC}");
        this->self.add_pattern(L"ratNONDEC", L"{intNONDEC}_{ADIGIT}+");
        this->self.add_pattern(L"IDENT", L"[A-Za-z][_A-Za-z0-9]*");
        this->self.add_pattern(L"INTERPRETED_STRING", 
          L"\\\"([^\\\"\\\\]|\\.)*\\\"");
        this->self.add_pattern(L"RAW_STRING", L"`[^`]*`");

        identifier = L"{IDENT}";
        constant_raw = L"{IDENT}?{RAW_STRING}";
        constant_interpreted = L"{IDENT}?{INTERPRETED_STRING}";
        integer = L"0|(~?({intDEC}|{intNONDEC}|{intUNARY}))";
        character = L"'([^'\\\\]|\\\\.)+'";

        dblslash = L"\\\\\\\\";
        range = L"\\.\\.";
        arrow = L"->";
        dblsemi = L";;";

        float_val = L"(0\\.0)|~?({floatDEC}|{floatNONDEC})";
        rational = L"(0_1)|(~?)({ratDEC}|{ratNONDEC})";

        this->self =
          spaces[lex::_pass = lex::pass_flags::pass_ignore]
        | if_
        | fi_
        | where_
        | then_
        | elsif_
        | true_
        | false_
        | identifier
        | constant_raw
          [
            detail::build_constant()
          ]
        | constant_interpreted
          [
            detail::build_constant()
          ]
        | character
          [
            detail::build_character()
          ]
        | integer[detail::build_integer()]
        | float_val[detail::build_float()]
        | rational[detail::build_rational()]
        | L':'
        | L'['
        | L']'
        | range
        | L'.'
        | L'='
        | L'&'
        | L'#'
        | L'@'
        | dblslash
        | L'\\'
        | L'('
        | L')'
        | arrow
        | L'|'
        | dblsemi
        ;

        #if 0
        this->self(L"CONSTANT_INTERPRETED") = 
            constant_interpreted
            [
              build_constant(m_constant_type, m_constant_value)
            ]
          | constant_interpreted_escaped
          | constant_interpreted_any
            [
              ph::ref(m_constant_value) +=
                ph::construct<std::wstring>(_start, _end),
              _pass = lex::pass_flags::pass_ignore
            ]
        ;

        this->self(L"CONSTANT_RAW") =
            constant_raw
            [
              build_constant(m_constant_type, m_constant_value)
            ]
          | constant_raw_any
            [
              ph::ref(m_constant_value) += 
                ph::construct<std::wstring>(_start, _end),
              _pass = lex::pass_flags::pass_ignore
            ]
        ;

        constant_raw = L"`";
        constant_raw_any = L".*";

        constant_interpreted = L"\\\"";
        constant_interpreted_escaped = L"\\t|\\a";
        constant_interpreted_any = L".*";
        #endif
      }

      lex::token_def<lex::unused_type, wchar_t> 
        if_, fi_, where_, then_, elsif_, true_, false_
      , spaces
      , arrow, dblsemi, dblslash, range
      ;

      lex::token_def<std::basic_string<wchar_t>, wchar_t>
        identifier
      ;

      lex::token_def<value_wrapper<mpz_class>, wchar_t> 
      //lex::token_def<mpz_class, wchar_t> 
        integer
      ;

      lex::token_def<value_wrapper<mpf_class>, wchar_t>
        float_val
      ;

      lex::token_def<value_wrapper<mpq_class>, wchar_t>
      //lex::token_def<mpq_class, wchar_t>
        rational
      ;

      lex::token_def<std::pair<std::wstring, std::wstring>, wchar_t>
      //these are the two that the parser should match
        constant_raw
      , constant_interpreted
      ;

      lex::token_def<char32_t, wchar_t> character;

      private:
      std::wstring m_constant_type;
      std::wstring m_constant_value;
    };
  }
}
