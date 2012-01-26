/* TransLucid lexer utility functions.
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

#include <tl/charset.hpp>
#include <tl/lexer_util.hpp>
#include <tl/parser_iterator.hpp>

#include <iostream>

namespace TransLucid { namespace Lexer {

//returns <valid, s>
//reads whole sequences of escape characters at a time, this is so that
//\x characters work
template <typename Iterator>
std::pair<bool, u32string>
build_escaped_characters
(
  Iterator& current,
  const Iterator& end
)
{
  std::string building;
  bool error = false;
  int to_read = 0;

  while (*current == '\\' && error == false)
  {
    ++current;
    char32_t c = *current;
    switch(c)
    {
      case 'U':
      to_read = 8;
      ++current;
      break;

      case 'u':
      to_read = 4;
      ++current;
      break;

      case 'x':
      to_read = 2;
      ++current;
      break;

      case 'n':
      building += "\n";
      ++current;
      break;

      case 'r':
      building += "\r";
      ++current;
      break;

      case 't':
      building += "\t";
      ++current;
      break;

      case '\'':
      building += "\'";
      ++current;
      break;

      case '\"':
      building += "\"";
      ++current;
      break;

      case '\\':
      building += "\\";
      ++current;
      break;

      default:
      //invalid control character
      error = true;
      break;
    }

    //read the requested number of characters
    std::string chars;
    if (to_read > 0)
    {
      while (to_read > 0 && current != end && *current != '\'')
      {
        c = *current;  
        if ((c >= '0' && c <= '9') || (c >= 'A' && c <= 'F'))
        {
          chars += c;
        }
        else
        {
          break;
        }
        --to_read;
        ++current;
      }
    }

    //if we didn't read everything then error
    if (to_read != 0)
    {
      error = true;
    }
    else
    {
      uint32_t value = 0;
      for (char c : chars)
      {
        if (c >= '0' && c <= '9')
        {
          value = value * 16 + (c - '0');
        }
        else
        {
          value = value * 16 + (c - 'A' + 10);
        }
      }

      //it was a byte if it was two characters, otherwise it was a whole
      //character
      //if (chars.length() == 2)
      //{
      //  building += char(value & 0xFF);
      //}
      //else
      {
        building += utf32_to_utf8(u32string(1, value));
      }
    }
  }

  u32string u32result = utf8_to_utf32(building);
  return std::make_pair(!error, u32result);
}

template 
std::pair<bool, u32string>
build_escaped_characters<Parser::U32Iterator>
(
  Parser::U32Iterator& begin,
  const Parser::U32Iterator& end
);

template 
std::pair<bool, u32string>
build_escaped_characters<Parser::PositionIterator<Parser::U32Iterator>>
(
  Parser::PositionIterator<Parser::U32Iterator>& begin,
  const Parser::PositionIterator<Parser::U32Iterator>& end
);

} }
