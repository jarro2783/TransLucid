/* Header type.
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

#if 0

#include <tl/header_type.hpp>
#include <tl/parser.hpp>

namespace TransLucid
{

size_t
HeaderType::hash() const
{
   return 0;
}

void
HeaderType::parseString
(
  const u32string& s,
  const Tuple& c,
  Interpreter& i
)
{
  #if 0
  Parser::HeaderGrammar hg(m_header, i.parsers());

  Parser::UIterator iter(s);

  Parser::iterator_t begin(Parser::Iterator(iter),
                           Parser::Iterator(iter.make_end()));

  i.parseRange(begin, Parser::iterator_t(), hg);
  #endif
}

void
HeaderType::parseFile
(
  const u32string& file,
  const Tuple& c,
  Interpreter& i
)
{
}

bool
HeaderType::operator==(const HeaderType& rhs) const
{
  return m_header == rhs.m_header;
}

}

#endif
