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
#include "tl/static_lexer.hpp"
#include "tl/lexertl.hpp"

namespace TransLucid
{

namespace Parser
{

namespace
{
  class ValueBuilder
  {
    private:
    typedef PositionIterator<U32Iterator> iterator;

    public:
    ValueBuilder()
    {
      for (int i = 0; i != TOKEN_LAST; ++i)
      {
        m_functions[i] = &buildEmpty;
      }

      m_functions[TOKEN_INTEGER] = &buildInteger;
    }

    template <typename Begin>
    TokenValue
    operator()
    (
      size_t index, 
      Begin&& begin,
      const iterator& end
    )
    {
      return (*m_functions[index])(std::forward<Begin>(begin), end);
    }

    private:
    typedef TokenValue (*build_func)(iterator begin, const iterator& end);
    build_func m_functions[TOKEN_LAST];

    static TokenValue
    buildEmpty(iterator begin, const iterator& end)
    {
      return nil();
    }

    static TokenValue
    buildInteger(iterator begin, const iterator& end)
    {
    }

  };

  static ValueBuilder build_value;
}

Lexer::Lexer()
{
}

Token
Lexer::next()
{
  translucid_lex(m_results);

  size_t id = m_results.id;

  if (id <= TOKEN_FIRST || id >= TOKEN_LAST)
  {
    //TODO error handling
    throw "Invalid token";
  }

  //build up the token
  TokenValue tokVal = build_value(id, m_results.start, m_results.end);
}

}

}
