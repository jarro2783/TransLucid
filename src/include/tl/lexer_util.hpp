/* TransLucid lexer utility functions.
   Copyright (C) 2011 Jarryd Beck and John Plaice

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
 * This file contains the following:
 * - init_*: initialises the numbers that can be recognised by the lexer.
 * - value_wrapper: a class which wraps values and acts as a proxy to
 * remove ambiguity when placing them in a boost::variant.
 */

#ifndef TL_LEXER_UTIL_HPP_INCLUDED
#define TL_LEXER_UTIL_HPP_INCLUDED

#include <gmpxx.h>

namespace TransLucid
{
  namespace Lexer
  {
    template <typename Iterator>
    mpq_class
    init_mpq
    (
      const Iterator& begin,
      const Iterator& end,
      int base
    );

    template <typename Iterator>
    mpf_class
    init_mpf
    (
      const Iterator& begin,
      const Iterator& end,
      int base
    );

    //returns <valid, str>
    template <typename Iterator>
    std::pair<bool, u32string>
    build_escaped_characters
    (
      Iterator& begin,
      const Iterator& end
    );

    template <typename T>
    struct value_wrapper;

    template <typename T>
    std::ostream&
    operator<<(std::ostream&, const value_wrapper<T>& rhs);

    template <typename T>
    struct value_wrapper
    {
      public:
      value_wrapper(const T& t)
      : m_t(t)
      {
      }

      value_wrapper()
      {
      }

      operator T const&() const
      {
        return m_t;
      }

      bool operator==(const value_wrapper<T>& rhs) const
      {
        return m_t == rhs.m_t;
      }

      friend 
      bool 
      operator==(const T& lhs, const value_wrapper<T>& rhs)
      {
        return lhs == rhs.m_t;
      }

      friend
      bool
      operator==(const value_wrapper<T>& lhs, const T& rhs)
      {
        return lhs.m_t == rhs;
      }

      friend
      std::ostream&
      operator<< <T>(std::ostream&, const value_wrapper<T>& rhs);

      private:
      T m_t;
    };

    template <typename T>
    bool
    operator==(const T& lhs, const value_wrapper<T>& rhs)
    {
      return lhs == rhs.m_t;
    }

    template <typename T>
    std::ostream&
    operator<<(std::ostream& os, const value_wrapper<T>& rhs)
    {
      os << rhs.m_t;
      return os;
    }
  }
}

#endif
