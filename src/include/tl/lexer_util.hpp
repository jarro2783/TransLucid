/* TransLucid lexer utility functions.
   Copyright (C) 2011--2013 Jarryd Beck

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

/**
 * @file lexer_util.hpp
 * Lexer utility header.
 */

#ifndef TL_LEXER_UTIL_HPP_INCLUDED
#define TL_LEXER_UTIL_HPP_INCLUDED

namespace TransLucid
{
  namespace Lexer
  {
    //returns <valid, str>
    template <typename Iterator>
    std::pair<bool, u32string>
    build_escaped_characters
    (
      Iterator& begin,
      const Iterator& end
    );
  }
}

#endif
