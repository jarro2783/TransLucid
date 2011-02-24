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
#include <tl/charset.hpp>

namespace boost { namespace spirit { namespace traits
{
  template <typename Iterator>
  struct assign_to_attribute_from_iterators<mpz_class, Iterator>
  {
    static void
    call(Iterator const& first, Iterator const& last, mpz_class& attr)
    {
      if (last - first == 1 && *first == L'0')
      {
        attr = 0;
      }
      else
      {
        //check for negative
        bool negative = false;
        Iterator current = first;
        if (*current == L'~')
        {
          negative = true;
          ++current;
        }

        if (*current == L'0')
        {
          //nondecint
          ++current;
          //this character is the base
          int base = *current;

          if (base >= '0' && base <= '9')
          {
            base = base - '0';
          }
          else if (base >= 'A' && base <= 'Z')
          {
            base = base - 'A' + 10;
          }
          else if (base >= 'a' && base <= 'z')
          {
            base = base - 'a' + 10 + 26;
          }

          //we are guaranteed to have at least this character
          ++current;
          if (base == 1)
          {
            attr = last - current;
          }
          else
          {
            attr = mpz_class(std::string(current, last), base);
          }
        }
        else
        {
          //decint
          //the lexer is guaranteed to have given us digits in the range
          //0-9 now
          attr = mpz_class(std::string(current, last), 10);
        }

        if (negative)
        {
          attr = -attr;
        }
      }
    }
  };
}}}

namespace TransLucid
{
  namespace Parser
  {
    namespace lex = boost::spirit::lex;

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
      , identifier(L"[A-Za-z][_A-Za-z0-9]*")
      {
        using boost::phoenix::ref;
        using lex::_val;
        using lex::_start;
        using lex::_end;
        namespace ph = boost::phoenix;

        this->self.add_pattern(L"DIGIT", L"[0-9]");
        this->self.add_pattern(L"ADIGIT", L"[0-9A-Za-z]");
        this->self.add_pattern(L"intDEC", L"[1-9]{DIGIT}*");
        this->self.add_pattern(L"intNONDEC", L"0[2-9A-Za-z]{ADIGIT}+");
        this->self.add_pattern(L"intUNARY", L"011+");

        integer = L"0|(~?({intDEC}|{intNONDEC}|{intUNARY}))";

        dblslash = L"\\\\\\\\";
        range = L"\\.\\.";
        arrow = L"->";
        dblsemi = L";;";

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
        | integer
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
      }

      lex::token_def<lex::unused_type, wchar_t> 
        if_, fi_, where_, then_, elsif_, true_, false_
      , spaces
      , arrow, dblsemi, dblslash, range
      ;

      lex::token_def<std::basic_string<wchar_t>, wchar_t>
        identifier
      ;

      lex::token_def<mpz_class, wchar_t> 
        integer
      ;
    };
  }
}
