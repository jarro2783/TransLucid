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
  return c == ' ' || c == '\t' || c == '\n';
}
 
u32string
LineTokenizer::next()
{
  try
  {
    m_state = READ_SKIP_SPACE;
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
          m_state = READ_SCANNING;
        }
      }
    }
    else
    {
      //otherwise get going on the next character
      ++m_current;
    }

    //are we still in skipping spaces mode?
    //if so, start reading and don't add to the buffer
    if (m_state == READ_SKIP_SPACE)
    {
      char32_t c = *m_current;

      while (is_space(c))
      {
        ++m_current;
        c = *m_current;
      }

      //the last one that failed needs to go onto the buffer
      m_line += c;
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

//upon entering this function, we will have skipped all the spaces at the
//start, and we will be at the first character which has already been added to
//the buffer
void
LineTokenizer::readOuter()
{
  //first we need to determine if we have a $$ or %%
  char32_t c = currentChar();
  bool done = false;

  if (c == '$')
  {
    c = nextChar();
    if (c == '$')
    {
      done = true;
    }
  }
  else if (c == '%')
  {
    c = nextChar();
    if (c == '%')
    {
      done = true;
    }
  }

  //do nothing if they didn't match, because the rest of the algorithm needs
  //to start with that character (it could be the first ; or a " or `)

  while (!done && m_current != Parser::U32Iterator())
  {
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
