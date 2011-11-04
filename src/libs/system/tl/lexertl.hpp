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

#ifndef TL_LEXERTL_HPP_INCLUDED
#define TL_LEXERTL_HPP_INCLUDED

#include "lexertl/lookup.hpp"
#include "tl/lexer_tokens.hpp"

#include <tl/ast-new.hpp>
#include <tl/context.hpp>
#include <tl/parser_iterator.hpp>
#include <tl/system.hpp>
#include <tl/variant.hpp>

#include <gmpxx.h>

namespace TransLucid
{
  namespace Parser
  {
    struct Position
    {
      u32string file;
      int line;
      int character;

      U32Iterator begin;
      U32Iterator end;
    };

    struct nil {};

    typedef Variant
    <
      nil,
      char32_t,
      TreeNew::InfixAssoc,
      TreeNew::UnaryType,
      u32string,
      mpz_class,
      std::pair<u32string, u32string>
    > TokenValue;

    class Token
    {
      public:
      template <typename Pos, typename Val>
      Token(Pos&& position, Val&& val, int type)
      : m_pos(std::forward<Pos>(position))
      , m_val(std::forward<Val>(val))
      , m_type(type)
      {
      }

      const Position&
      getPosition() const
      {
        return m_pos;
      }

      const TokenValue&
      getValue() const
      {
        return m_val;
      }

      int
      getType() const
      {
        return m_type;
      }

      private:
      Position m_pos;
      TokenValue m_val;
      int m_type;
    };

    class Lexer
    {
      public:
      typedef PositionIterator<U32Iterator> iterator;

      Lexer() = default;

      //get the next token
      Token
      next
      (
        iterator& begin, 
        const iterator& end,
        Context& context,
        System::IdentifierLookup& idents
      );
    };
  }
}

#endif
