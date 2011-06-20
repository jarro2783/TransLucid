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

#include <deque>

#include <tl/parser_iterator.hpp>

namespace TransLucid
{
  class LineTokenizer
  {
    public:
    //construct with an iterator
    LineTokenizer(TransLucid::Parser::U32Iterator& begin)
    : m_current(begin)
    {
    }

    u32string next();

    private:

    enum State
    {
      READ_SCANNING,
      READ_RAW,
      READ_INTERPRETED,
      READ_SEMI
    };

    /**
     * Reads a line up to the next ;; at the outer level of nesting of various
     * gizmos.
     */
    void
    readOuter();

    void
    readRawString();

    void
    readInterpretedString();

    char32_t
    nextChar();

    std::deque<char32_t> m_lookahead;
    Parser::U32Iterator& m_current;
    Parser::U32Iterator m_end;
    u32string m_line;
    State m_state;
  };
}

#endif
