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
#include <tl/charset.hpp>

namespace TransLucid
{
  namespace Parser
  {
    namespace lex = boost::spirit::lex;

    template <typename Lexer>
    struct lex_tl_tokens : lex::lexer<Lexer>
    {
      lex_tl_tokens()
      : any(L".")
      , if_(L"if")
      , fi(L"fi")
      , where(L"where")
      , then(L"when")
      , elseif(L"elsif")
      , true_(L"true")
      , false_(L"false")
      , identifier(L"[A-Za-z][_A-Za-z0-9]*")
      , nondecint(L"0[2-9A-Za-z][0-9A-Za-z]+")
      {
        using boost::phoenix::ref;
        namespace ph = boost::phoenix;

        this->self =
          any[lex::_pass = lex::pass_flags::pass_ignore]
        | if_
        | fi
        | where
        | then
        | elsif
        | true_
        | false_
        | identifier
        | nondecint
        ;
    }

      lex::token_def<lex::unused_type, wchar_t> 
        if_
      , fi
      , any
      , identifier
      ;

      lex::token_def<mpz_class, wchar_t> nondecint;
    };
  }
}
