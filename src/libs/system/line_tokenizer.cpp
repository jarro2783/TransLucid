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
 
u32string
LineTokenizer::next()
{
  try
  {
    m_state = READ_SCANNING;
    m_line.clear();

    if (m_first)
    {
      m_first = false;

      if (m_current != m_end)
      {
        m_line += *m_current;
      }
    }
    else
    {
      nextChar();
    }

    readOuter();
  } 
  catch (EndOfInput&)
  {
    //end of input whilst reading line
  }
  catch (NewLineInString&)
  {
    //new line token found in string
  }
  
  return m_line;
}

void
LineTokenizer::readOuter()
{
  bool done = false;
  while (!done && m_current != Parser::U32Iterator())
  {
    char32_t c = currentChar();
    switch (c)
    {
      case ';':
      m_state = READ_SEMI;
      {
        c = nextChar();
        if (c == ';')
        {
          done = true;
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
}

void
LineTokenizer::readRawString()
{
  //read until another "`"
  m_state = READ_RAW;

  char32_t c = nextChar();
  while (c != '`')
  {
    c = nextChar();
  }

  m_state = READ_SCANNING;
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
  m_line += c;
  return c;
}

void
LineTokenizer::readInterpretedString()
{
  //read until ", but skip backslash and complain about new lines
  m_state = READ_INTERPRETED;
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

  m_state = READ_SCANNING;
}

}
