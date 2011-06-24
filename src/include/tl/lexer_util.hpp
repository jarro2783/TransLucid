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
    enum OpTokens
    {
      TOK_BINARY_OP = 1000,
      TOK_PREFIX_OP,
      TOK_POSTFIX_OP
    };

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

    /**
     * Wraps a value for use in a boost variant so that the constructors are
     * not ambiguous.
     */
    template <typename T>
    struct value_wrapper
    {
      public:
      /**
       * Constructs a value_wrapper with a value.
       * @param t The value to hold.
       */
      value_wrapper(const T& t)
      : m_t(t)
      {
      }

      /**
       * Constructs a value_wrapper with the default value of the stored type.
       */
      value_wrapper()
      {
      }

      /**
       * Convert to the stored type.
       * @return A reference to the stored type.
       */
      operator T const&() const
      {
        return m_t;
      }

      /**
       * Test equality with another wrapper of the same type.
       * @param rhs The value to test equality with.
       * @return True if the stored value == the rhs stored value.
       */
      bool operator==(const value_wrapper<T>& rhs) const
      {
        return m_t == rhs.m_t;
      }

      /**
       * Check equality with a non wrapped value.
       * @param lhs The non wrapped value.
       * @param rhs The wrapped value.
       * @return True if lhs == the stored value of rhs.
       */
      friend 
      bool 
      operator==(const T& lhs, const value_wrapper<T>& rhs)
      {
        return lhs == rhs.m_t;
      }

      /**
       * Check equality with a non wrapped value.
       * @param lhs The wrapped value.
       * @param rhs The non wrapped value.
       * @return True if the stored value of lhs == rhs.
       */
      template <typename S>
      friend
      bool
      operator==(const value_wrapper<S>& lhs, const S& rhs);
      #if 0
      {
        return lhs.m_t == rhs;
      }
      #endif

      /**
       * Print a value.
       * @param os The ostream to print to.
       * @param rhs The object to print.
       * @return The passed ostream.
       */
      friend
      std::ostream&
      operator<< <T>(std::ostream& os, const value_wrapper<T>& rhs);

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
