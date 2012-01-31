/* Line tokenizer.
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

#ifndef LINE_TOKENIZER_HPP_INCLUDED
#define LINE_TOKENIZER_HPP_INCLUDED

#include <tl/parser_iterator.hpp>
#include <deque>

namespace TransLucid
{
  enum class LineType
  {
    LINE,
    DOUBLE_DOLLAR,
    DOUBLE_PERCENT,
    EMPTY
  };

  class LineTokenizer
  {
    private:

    enum class State
    {
      READ_SCANNING,
      READ_RAW,
      READ_INTERPRETED,
      READ_SEMI,
      READ_SKIP_SPACE,
    };

    public:

    struct Line
    {
      int character;
      int line;
      u32string text;
      LineType type;
    };

    //construct with an iterator
    LineTokenizer(TransLucid::Parser::U32Iterator& begin,
      const TransLucid::Parser::U32Iterator& end
    )
    : m_current(begin)
    , m_end(end)
    , m_state(State::READ_SCANNING)
    , m_first(true)
    , m_whereDepth(0)
    , m_readingIdent(false)
    , m_lineCount(0)
    , m_charCount(0)
    , m_peeked(0)
    {
    }

    //std::pair<LineType, u32string> next();
    Line next();

    bool
    end() const
    {
      return m_current == m_end;
    }

    void
    startChar(int c);

    void
    startLine(int line);

    private:

    /**
     * Reads a line up to the next ;; at the outer level of nesting of various
     * gizmos.
     */
    LineType
    readOuter();

    void
    preLineSkip();

    void
    readRawString();

    void
    readInterpretedString();

    char32_t
    nextChar();

    char32_t
    nextCharCommon();

    char32_t
    nextCharDiscard();

    char32_t
    currentChar();

    void
    skipToNewline();

    //peeks at the next character and stores it in a buffer so that
    //nextChar still gives the first one before the peeking
    char32_t peek();

    Parser::U32Iterator& m_current;
    Parser::U32Iterator m_end;
    char32_t m_currentChar;
    u32string m_line;
    State m_state;
    bool m_first;

    int m_whereDepth;
    bool m_readingIdent;
    u32string m_currentIdent;
    int m_lineCount;
    int m_charCount;

    int m_startLine;
    int m_startChar;

    std::deque<char32_t> m_peek;
    size_t m_peeked;
  };

  std::ostream&
  operator<<(std::ostream& os, LineType l);
}

#endif
