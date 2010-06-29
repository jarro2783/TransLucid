/* Parser forward declarations.
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

#ifndef PARSER_ITERATOR_HPP_INCLUDED
#define PARSER_ITERATOR_HPP_INCLUDED

#include <iterator>

namespace TransLucid
{
  namespace Parser
  {
    class U32Iterator : 
      public std::iterator
      <
        std::input_iterator_tag,
        char32_t,
        size_t,
        char32_t*,
        char32_t&
      >
    {
      public:

      U32Iterator();

      U32Iterator& operator++();

      U32Iterator operator++(int);
    };
  } //namespace Parser
} //namespace TransLucid

#endif
