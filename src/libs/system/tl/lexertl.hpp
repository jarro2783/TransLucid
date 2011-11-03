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

#include "lexertl/lookup.hpp"
#include <tl/parser_iterator.hpp>
#include <tl/variant.hpp>
#include <gmpxx.h>
#include "tl/lexer_tokens.hpp"

namespace TransLucid
{
  namespace Parser
  {
    class Position
    {
      u32string m_file;
      int m_line;
      int m_character;

      U32Iterator m_begin;
      U32Iterator m_end;
    };

    class Token
    {
      public:

      private:
      Position m_pos;
    };

    struct nil {};

    typedef Variant
    <
      nil,
      char32_t,
      u32string,
      mpz_class,
      std::pair<u32string, u32string>
    > TokenValue;

    class Lexer
    {
      public:

      Lexer();

      //get the next token
      Token
      next();

      private:
      typedef PositionIterator<U32Iterator> iterator;
      lexertl::basic_match_results<iterator, size_t> m_results;
    };
  }
}
