/* Character set conversion.
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

#ifndef CHARSET_HPP_INCLUDED
#define CHARSET_HPP_INCLUDED

#include <tl/types.hpp>
#include <iconv.h>

/**
 * @file charset.hpp
 * Character set utility functions. Contains functions for converting
 * between character sets.
 */

namespace TransLucid
{
  /**
   * An iconv wrapper class. Provides a C++ interface to the iconv
   * library.
   */
  class Iconv
  {
    public:
    /**
     * Construct an Iconv object, the to and from character sets must be
     * provided.
     * @param to The character set to convert to.
     * @param from The character set to convert from.
     * @todo What is the best way to handle errors here?
     */
    Iconv(const char* to, const char* from)
    {
      m_iconv = iconv_open(to, from);
      if (m_iconv == (iconv_t)-1) 
      {
        throw std::string("Failed to initialise iconv");
      }
    }

    ~Iconv()
    {
      iconv_close(m_iconv);
    }

    /**
     * Do the conversion directly calling ::iconv.
     * @param inbuf The input array.
     * @param inbytesleft The number of bytes left to read in the input.
     * @param outbuf The output array.
     * @param outbytesleft The amount of space left in the output buffer.
     * @return The number of characters converted nonreversibly.
     */
    size_t
    iconv(char** inbuf, size_t* inbytesleft,
          char** outbuf, size_t* outbytesleft)
    {
      return ::iconv(m_iconv, inbuf, inbytesleft, outbuf, outbytesleft);
    }

    private:
    iconv_t m_iconv;
  };

  /**
   * Converts a UTF-32 string to a UTF-8 string.
   * @param s The UTF-32 string.
   * @return The equivalent UTF-8 string.
   */
  std::string
  utf32_to_utf8(const u32string& s);

  /**
   * Converts a UTF-8 string to a UTF-32 string.
   * @param s The UTF-8 string.
   * @return The equivalent UTF-32 string.
   */
  std::u32string
  utf8_to_utf32(const std::string& s);

  /**
   * Converts a UTF-32 string to ASCII. All the characters must be 
   * representable in ASCII. If any are not, an exception is thrown.
   * @param s The UTF-32 string.
   * @return The string in ASCII.
   * @throws char* "character not ascii"
   */
  std::string
  u32_to_ascii(const u32string& s);

  template <typename T>
  u32string
  to_u32string(const T& s)
  {
    return u32string(s.begin(), s.end());
  }

  template <typename T>
  std::basic_string<uint32_t>
  to_unsigned_u32string(const T& s)
  {
    return std::basic_string<uint32_t>(s.begin(), s.end());
  }

  template <typename T>
  std::basic_string<uint32_t>
  chars_to_unsigned_u32string(T* s)
  {
    std::basic_string<uint32_t> ustring;
    for (auto i = s; *i != 0; ++i)
    {
      ustring += *i; 
    }
    return ustring;
  }

  inline bool 
  validate_uchar(char32_t c)
  {
    if (c < 0xd800)
      return true;
    if (c < 0xe000)
      return false;
    if (c < 0x110000)
      return true;
    return false;
  }

  inline bool 
  validate_ustring(const u32string& s) {
    size_t i = 0;
    bool valid = true;
    while (i != s.size() && valid)
    {
      valid = validate_uchar(s[i]);
      ++i;
    }
    return valid;
  }

  inline
  std::basic_string<unsigned int>
  literal(const char* s)
  {
    std::basic_string<unsigned int> dest;
    const char* c = s;
    while (*c != 0)
    {
      dest += *c;
      ++c;
    }

    return dest;
  }

  inline
  std::basic_string<unsigned int>
  literal(char c)
  {
    return std::basic_string<unsigned int>(1, static_cast<unsigned int>(c));
  }
}

namespace std
{
  inline ostream&
  operator<<(ostream& os, const u32string& s)
  {
    os << TransLucid::utf32_to_utf8(s);
    return os;
  }

  inline ostream&
  operator<<(ostream& os, const std::basic_string<unsigned int>& s)
  {
    u32string u32s(s.begin(), s.end());
    os << u32s;
    return os;
  }
}

#endif // CHARSET_HPP_INCLUDED
