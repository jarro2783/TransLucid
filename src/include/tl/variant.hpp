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

#include <new>
#include <type_traits>
#include <utility>

#include <tl/mpl.hpp>

namespace TransLucid
{
  template <typename First, typename... Types>
  class Variant
  {
    private:

    //static constexpr alignment = 

    template <typename T>
    struct SizeofPlusAlignof
    {
      static constexpr size_t value = sizeof(T) + alignof(T);
    };

    //size = max of (size + alignment) of each thing
    static constexpr size_t size = 
      max
      <
        SizeofPlusAlignof,
        First,
        Types...
      >::value;

    public:

    Variant()
    {
      //try to construct First
      //if this fails then First is not default constructible
      construct(First());
      indicate_which(0);
    }

    ~Variant();

    template <typename T>
    Variant(const T& t);

    Variant(const Variant& rhs);

    Variant(Variant&& rhs);

    Variant& operator=(const Variant& rhs);

    Variant& operator=(Variant&& rhs);

    int which() {return m_which;}

    private:

    //TODO implement with alignas when it is implemented in gcc
    union
    {
      char m_storage[size]; //max of size + alignof for each of Types...
      int m_align; //the type with the max alignment
    };

    int m_which;

    void indicate_which(int which) {m_which = which;}

    template <typename T>
    void
    construct(T t)
    {
      new(m_storage) T(std::forward(t));
    }
  };
}
