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

#include <tl/line_tokenizer.hpp>

namespace TransLucid
{

namespace
{
  struct EndOfInput
  {
  };

  struct NewLineInString
  {
  };
}

inline bool
is_space(char32_t c)
{
  return c == ' ' || c == '\t' || c == '\n' || c == '\r';
}

inline bool
is_ident_begin(char32_t c)
{
  return (c >= 'A' && c <= 'Z')
    || (c >= 'a' && c <= 'z')
    || (c == '_');
}

inline bool
is_ident_inside(char32_t c)
{
  return is_ident_begin(c) || (c >= '0' && c <= '9');
}
 
std::pair<LineType, u32string>
LineTokenizer::next()
{
  LineType type = LineType::LINE;

  try
  {
    m_state = State::READ_SKIP_SPACE;
    m_line.clear();

    //handle some special cases first

    //the first time we tokenize something we are already pointing at
    //the first character
    if (m_first)
    {
      m_first = false;

      //but we have to make sure that there is actually some input
      if (m_current != m_end)
      {
        //if we see a space then do nothing
        //otherwise start the reading
        char32_t c = *m_current;
        if (!is_space(c))
        {
          m_line += *m_current;
          m_state = State::READ_SCANNING;
        }
      }
      else
      {
        throw EndOfInput();
      }
    }
    else
    {
      //otherwise get going on the next character
      ++m_current;

      //make sure we didn't get to the end
      if (m_current == m_end)
      {
        throw EndOfInput();
      }
    }

    //are we still in skipping spaces mode?
    //if so, start reading and don't add to the buffer
    if (m_state == State::READ_SKIP_SPACE)
    {
      char32_t c = *m_current;

      while (is_space(c))
      {
        ++m_current;

        if (m_current == m_end)
        {
          throw EndOfInput();
        }

        c = *m_current;
      }

      //the last one that failed needs to go onto the buffer
      m_line += c;
    }

    type = readOuter();
  } 
  catch (EndOfInput&)
  {
    //end of input whilst reading line

    //if we are still skipping spaces then the line is empty
    if (m_state == State::READ_SKIP_SPACE)
    {
      type = LineType::EMPTY;
    }
  }
  catch (NewLineInString&)
  {
    //new line token found in string
  }
  
  return std::make_pair(type, m_line);
}

//upon entering this function, we will have skipped all the spaces at the
//start, and we will be at the first character which has already been added to
//the buffer
LineType
LineTokenizer::readOuter()
{
  //first we need to determine if we have a $$ or %%
  LineType type = LineType::LINE;
  char32_t c = currentChar();
  bool done = false;

  if (c == '$')
  {
    c = nextChar();
    if (c == '$')
    {
      done = true;
      type = LineType::DOUBLE_DOLLAR;
    }
  }
  else if (c == '%')
  {
    c = nextChar();
    if (c == '%')
    {
      done = true;
      type = LineType::DOUBLE_PERCENT;
    }
  }

  //do nothing if they didn't match, because the rest of the algorithm needs
  //to start with that character (it could be the first ; or a " or `)

  while (!done && m_current != m_end)
  {
    if (m_readingIdent)
    {
      if (is_ident_inside(c))
      {
        m_currentIdent += c;
      }
      else if (c != '"' && c != '`')
      {
        if (m_currentIdent == U"where")
        {
          ++m_whereDepth;
        }
        else if (m_currentIdent == U"end")
        {
          --m_whereDepth;
        }

        m_readingIdent = false;
      }
    }
    else
    {
      if (is_ident_begin(c))
      {
        m_currentIdent.clear();
        m_currentIdent += c;
        m_readingIdent = true;
      }
    }

    switch (c)
    {
      case ';':
      m_state = State::READ_SEMI;
      {
        c = nextChar();
        if (c == ';' && m_whereDepth == 0)
        {
          done = true;
          m_state = State::READ_SCANNING;
        }
      }
      break;

      case '/':
      {
        c = nextChar();
        if (c == '/')
        {
          skipToNewline();
        }
      }
      break;

      case '"':
      readInterpretedString();
      break;

      case '`':
      readRawString();
      break;

      default:
      

      break;
    }

    if (!done)
    {
      c = nextChar();
    }
  }

  return type;
}

void
LineTokenizer::readRawString()
{
  //read until another "`"
  m_state = State::READ_RAW;

  char32_t c = nextChar();
  while (c != '`')
  {
    c = nextChar();
  }

  m_state = State::READ_SCANNING;
}

char32_t
LineTokenizer::currentChar()
{
  return *m_current;
}

char32_t
LineTokenizer::nextChar()
{
  ++m_current;

  if (m_current == m_end)
  {
    throw EndOfInput();
  }

  char32_t c = *m_current;

  if (c == '\n')
  {
    ++m_lineCount;
    m_charCount = 0;
  }
  else
  {
    ++m_charCount;
  }

  m_line += c;
  return c;
}

void
LineTokenizer::skipToNewline()
{
  char32_t c = nextChar();
  while (c != '\n')
  {
    c = nextChar();
  }
}

void
LineTokenizer::readInterpretedString()
{
  //read until ", but skip backslash and complain about new lines
  m_state = State::READ_INTERPRETED;
  char32_t c = nextChar();
  while (c != '"')
  {
    if (c == '\\')
    {
      nextChar();
      c = nextChar();
    }
    else if (c == '\n')
    {
      //pretend that the line ends here
      throw NewLineInString();
    }
    else
    {
      c = nextChar();
    }
  }

  m_state = State::READ_SCANNING;
}

std::ostream&
operator<<(std::ostream& os, LineType l)
{
  switch (l)
  {
    case LineType::LINE:
    os << "LineType::LINE";
    break;

    case LineType::DOUBLE_DOLLAR:
    os << "LineType::DOUBLE_DOLLAR";
    break;

    case LineType::DOUBLE_PERCENT:
    os << "LineType::DOUBLE_PERCENT";
    break;

    case LineType::EMPTY:
    os << "LineType::EMPTY";
    break;
  }
  return os;
}

} //namespace TransLucid
