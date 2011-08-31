/* Printing things.
   Copyright (C) 2009, 2010, 2011 Jarryd Beck and John Plaice

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

//if you need to print anything, include this, it's all here
//otherwise don't to save on unnecessary includes.

/** @file output.hpp
 * Definitions for outputting things.
 */

#ifndef TL_OUTPUT_HPP_INCLUDED
#define TL_OUTPUT_HPP_INCLUDED

#include <iostream>
#include <string>

#include <tl/charset.hpp>
#include <tl/types.hpp>
#include <tl/uuid.hpp>

namespace boost
{
  namespace uuids
  {
    std::ostream&
    operator<<(std::ostream& os, const uuid& id);
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
  operator<<(ostream& os, const basic_string<unsigned int>& s)
  {
    u32string u32s(s.begin(), s.end());
    os << u32s;
    return os;
  }
}

#endif