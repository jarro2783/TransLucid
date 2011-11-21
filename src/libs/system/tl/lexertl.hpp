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

#include <list>
#include <memory>

#include <gmpxx.h>

#include "lexertl/lookup.hpp"
#include "tl/lexer_tokens.hpp"

#include <tl/ast.hpp>
#include <tl/context.hpp>
#include <tl/parser_api.hpp>
#include <tl/parser_iterator.hpp>
#include <tl/system.hpp>
#include <tl/variant.hpp>

namespace TransLucid
{
  namespace Parser
  {
    struct nil {};

    typedef Variant
    <
      nil,
      char32_t,
      Tree::InfixAssoc,
      Tree::UnaryType,
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

      bool
      operator==(int id) const
      {
        return id == m_type;
      }

      bool
      operator!=(int id) const
      {
        return !operator==(id);
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

    //get the next token
    Token
    nextToken
    (
      StreamPosIterator& begin, 
      const StreamPosIterator& end,
      Context& context,
      System::IdentifierLookup& idents
    );
    
    class LexerIterator
    {
      public:

      LexerIterator
      (
        StreamPosIterator& begin,
        const StreamPosIterator& end,
        Context& context,
        const System::IdentifierLookup& idents
      )
      : m_stream(new std::list<Token>)
      , m_next(&begin), m_end(&end), m_context(&context), m_idents(idents)
      {
        //initialise position to the end, then try to read something
        m_pos = m_stream->end();
        readOne();
      }

      LexerIterator(const LexerIterator& rhs) = default;

      LexerIterator&
      operator++()
      {
        //the iterator will always point to something valid in the list
        ++m_pos;

        //if we were at the end however, get the next token and insert it
        //into the list
        if (m_pos == m_stream->end())
        {
          readOne();
        }

        return *this;
      }

      const Token&
      operator*() const
      {
        if (m_pos != m_stream->end())
        {
          return *m_pos;
        }
        else
        {
          return m_endToken;
        }
      }

      const Token*
      operator->() const
      {
        return &operator*();
      }

      bool
      operator==(const LexerIterator& rhs) const
      {
        return m_stream.get() == rhs.m_stream.get()
          && m_pos == rhs.m_pos
        ;
      }

      bool
      operator!=(const LexerIterator& rhs) const
      {
        return !operator==(rhs);
      }

      LexerIterator
      makeEnd() const
      {
        return LexerIterator(m_stream);
      }

      const Position&
      getPos() const
      {
        //TODO we probably want a proper end pos for each stream
        return operator*().getPosition();
      }

      private:
      typedef std::list<Token> TokenStream;
      typedef std::shared_ptr<TokenStream> TokenStreamPtr;

      //makes an end iterator, we still need to point to a stream
      LexerIterator(const TokenStreamPtr& s)
      : m_stream(s), m_pos(m_stream->end()), m_next(0), m_end(0), m_context(0)
      {
      }

      void
      readOne();

      TokenStreamPtr m_stream;
      TokenStream::iterator m_pos;

      StreamPosIterator* m_next;
      const StreamPosIterator* m_end;
      Context* m_context;
      System::IdentifierLookup m_idents;

      static Token m_endToken;
    };
  }
}

#endif
