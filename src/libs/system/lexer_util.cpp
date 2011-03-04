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

#include <iostream>

namespace TransLucid { namespace Lexer {

template <typename Iterator>
mpq_class
init_mpq(const Iterator& begin, const Iterator& end, int base)
{
  std::string s;
  s.reserve(end - begin);

  Iterator current = begin;
  while (current != end)
  {
    if (*current == '_')
    {
      s += '/';
    }
    else
    {
      s += *current;
    }
    ++current;
  }

  mpq_class value(s, base);

  return value;
}

template <typename Iterator>
mpf_class
init_mpf(const Iterator& begin, const Iterator& end, int base)
{
  unsigned int prec = mpf_get_default_prec();
  std::cerr << "default precision is " << prec << std::endl;
  Iterator current = begin;
  std::string s;
  s.reserve(end - begin);

  bool find_precision = false;
  while (current != end && !find_precision)
  {
    if (*current == '^')
    {
      s += '@';
    }
    else if (*current == '#')
    {
      find_precision = true;
    }
    else
    {
      s += *current;
    }
    ++current;
  }

  if (find_precision)
  {
    mpz_class precision(std::string(current, end), base);
    prec = precision.get_si();
  }

  std::cerr << "init_mpf: " << s << ", base: " << base 
    << ", precision " << prec << std::endl;
  return mpf_class(s, prec, base);
}

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
  std::cerr << "got the following string to escape: " <<
    u32string(current, end) 
    << std::endl;
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

      case 'a':
      building += "\a";
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
      std::cerr << "reading " << to_read << " characters" << std::endl;
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
        std::cerr << "value = " << value << std::endl;
      }

      //it was a byte if it was two characters, otherwise it was a whole
      //character
      if (chars.length() == 2)
      {
        std::cerr << "the byte is " << value << std::endl;
        building += char(value & 0xFF);
      }
      else
      {
        building += utf32_to_utf8(u32string(1, value));
      }
    }
  }

  std::cerr << "building = " << building << std::endl;

  std::cerr << "error: " << error << std::endl;
  std::cerr << "current ended on " << *current << std::endl;
  std::cerr << "remaining string = \"" << u32string(current, end) << "\""
    << std::endl;
  u32string u32result = utf8_to_utf32(building);
  std::cerr << "returning: \"" << u32result << "\"" << std::endl;
  return std::make_pair(!error, u32result);
}

template mpf_class init_mpf<std::wstring::const_iterator>
(
  const std::wstring::const_iterator&,
  const std::wstring::const_iterator&,
  int
);

#if 0
template mpf_class init_mpf<std::wstring::iterator>
(
  const std::wstring::iterator&,
  const std::wstring::iterator&,
  int
);
#endif

template mpq_class init_mpq<std::wstring::const_iterator>
(
  const std::wstring::const_iterator&,
  const std::wstring::const_iterator&,
  int
);

#if 0
template mpq_class init_mpq<std::wstring::iterator>
(
  const std::wstring::iterator&,
  const std::wstring::iterator&,
  int
);
#endif

template 
std::pair<bool, u32string>
build_escaped_characters<std::wstring::const_iterator>
(
  std::wstring::const_iterator& begin,
  const std::wstring::const_iterator& end
);
 
} }
