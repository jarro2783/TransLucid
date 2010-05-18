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

#ifndef CONSTHD_HPP_INCLUDED
#define CONSTHD_HPP_INCLUDED

#include <tl/hyperdaton.hpp>

namespace TransLucid 
{
  namespace Hyperdatons
  {
    class TypeConstHD : public HD
    {
      public:

      TypeConstHD(size_t value)
      : m_value(value)
      {}

      TaggedConstant
      operator()(const Tuple& k);

      private:
      size_t m_value;
    };

    class BoolConstHD : public HD
    {
      public:

      BoolConstHD(bool value)
      : m_value(value)
      {}

      TaggedConstant
      operator()(const Tuple& k);

      private:
      bool m_value;
    };

    class SpecialConstHD : public HD
    {
      public:
      SpecialConstHD(Special::Value v)
      : m_value(v)
      {}

      TaggedConstant
      operator()(const Tuple& k);

      private:
      Special::Value m_value;
    };

    class IntmpConstHD : public HD
    {
      public:
      IntmpConstHD(const mpz_class& value)
      : m_value(value)
      {}

      TaggedConstant
      operator()(const Tuple& k);

      private:
      mpz_class m_value;
    };

    class UCharConstHD : public HD
    {
      public:
      UCharConstHD(char32_t c)
      : m_value(c)
      {}

      TaggedConstant
      operator()(const Tuple& k);

      private:
      char32_t m_value;
    };

    class UStringConstHD : public HD
    {
      public:
      UStringConstHD(const u32string& s)
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
