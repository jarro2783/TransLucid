/* Hyperdatons which return constants.
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

#ifndef CONSTWS_HPP_INCLUDED
#define CONSTWS_HPP_INCLUDED

#include <tl/hyperdaton.hpp>

namespace TransLucid 
{
  /** 
   * @brief All of the hyperdatons.
   *
   * The namespace that contains all of the hyperdatons which are
   * in the system.
   */
  namespace Hyperdatons
  {
    class TypeConstWS : public WS
    {
      public:

      TypeConstWS(size_t value)
      : m_value(value)
      {}

      TaggedConstant
      operator()(const Tuple& k);

      private:
      size_t m_value;
    };

    /**
     * @brief Constant boolean.
     *
     * Returns the same boolean no matter what the context.
     */
    class BoolConstWS : public WS
    {
      public:

      BoolConstWS(bool value)
      : m_value(value)
      {}

      TaggedConstant
      operator()(const Tuple& k);

      private:
      bool m_value;
    };

    class SpecialConstWS : public WS
    {
      public:
      SpecialConstWS(Special::Value v)
      : m_value(v)
      {}

      TaggedConstant
      operator()(const Tuple& k);

      private:
      Special::Value m_value;
    };

    class IntmpConstWS : public WS
    {
      public:
      IntmpConstWS(const mpz_class& value)
      : m_value(value)
      {}

      TaggedConstant
      operator()(const Tuple& k);

      private:
      mpz_class m_value;
    };

    class UCharConstWS : public WS
    {
      public:
      UCharConstWS(char32_t c)
      : m_value(c)
      {}

      TaggedConstant
      operator()(const Tuple& k);

      private:
      char32_t m_value;
    };

    class UStringConstWS : public WS
    {
      public:
      UStringConstWS(const u32string& s)
      : m_value(s)
      {}

      TaggedConstant
      operator()(const Tuple& k);

      private:
      u32string m_value;
    };
  }
}

#endif
