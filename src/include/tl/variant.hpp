/* A tagged union variant class.
   Copyright (C) 2011 Jarryd Beck

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
 * @file system.hpp
 * A tagged union. This effectively has the same functionality as
 * boost::variant, but replaces it with C++11 features.
 */

#include <type_traits>

namespace TransLucid
{
  template <typename First, typename... Types>
  class Variant
  {
    public:

    Variant();

    ~Variant();

    template <typename T>
    Variant(const T& t);

    Variant(const Variant& rhs);

    Variant(Variant&& rhs);

    int which() {return m_which;}

    private:

    //TODO implement with alignas when it is implemented in gcc
    //char m_storage[]; //max of size + alignof for each of Types...

    int m_which;
  };
}
