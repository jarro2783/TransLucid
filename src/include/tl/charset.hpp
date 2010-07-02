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

namespace TransLucid
{
  class Iconv
  {
    public:
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

    size_t
    iconv(char** inbuf, size_t* inbytesleft,
          char** outbuf, size_t* outbytesleft)
    {
      return ::iconv(m_iconv, inbuf, inbytesleft, outbuf, outbytesleft);
    }

    private:
    iconv_t m_iconv;
  };

  std::string
  utf32_to_utf8(const u32string& s);

  std::u32string
  utf8_to_utf32(const std::string& s);

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
    return std::basic_string<unsigned int>(c, 1);
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
