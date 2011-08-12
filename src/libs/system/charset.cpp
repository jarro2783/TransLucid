/* Character set conversion.
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

#include <tl/charset.hpp>

#include <string>

namespace TransLucid
{

Iconv::Iconv(const char* to, const char* from)
{
  m_iconv = iconv_open(to, from);
  if (m_iconv == (iconv_t)-1) 
  {
    throw std::string("Failed to initialise iconv");
  }
}

Iconv::~Iconv()
{
  iconv_close(m_iconv);
}

}
