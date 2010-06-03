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

namespace TransLucid
{
  std::string
  utf32_to_utf8(const u32string& s);

  std::string
  u32_to_ascii(const u32string& s);

  template <typename T>
  u32string
  to_u32string(const T& s)
  {
    return u32string(s.begin(), s.end());
  }

  inline bool 
  validate_uchar(char32_t c)
  {
    #warning check that it is a valid uchar
    return true;
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
}

namespace std
{
  inline ostream&
  operator<<(ostream& os, const u32string& s)
  {
    os << TransLucid::utf32_to_utf8(s);
    return os;
  }
}

#endif // CHARSET_HPP_INCLUDED
