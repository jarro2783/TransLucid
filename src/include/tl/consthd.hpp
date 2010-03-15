/* TODO: Give a descriptor.
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
#include <tl/interpreter.hpp>
#include <tl/builtin_types.hpp>
#include <tl/fixed_indexes.hpp>

namespace TransLucid
{
  namespace ConstHD
  {

    class ExprHD : public HD
    {
      public:
      ExprHD(HD* system)
      : m_system(system)
      {}

      virtual uuid
      addExpr(const Tuple& k, HD* h)
      {
        return nil_uuid();
      }

      protected:
      HD* m_system;
    };

    class Intmp : public ExprHD
    {
      public:
      static const char32_t* name;

      Intmp(HD* system)
      : ExprHD(system)
      {}

      TaggedValue
      operator()(const Tuple& k);

      private:
    };

    class UChar : public ExprHD
    {
      public:
      static const char32_t* name;

      UChar(HD* system)
      : ExprHD(system)
      {}

      TaggedValue
      operator()(const Tuple& k);
    };

    class UString : public ExprHD
    {
      public:
      static const char32_t* name;

      UString(HD* system)
      : ExprHD(system)
      {}

      TaggedValue
      operator()(const Tuple& k);
    };

    //this is not the intmp builder, this returns a constant intmp
    class IntmpConst : public HD
    {
      public:
      IntmpConst(const mpz_class& v)
      : m_v(v)
      {}

      TaggedValue
      operator()(const Tuple& k)
      {
        return TaggedValue(TypedValue(TransLucid::Intmp(m_v),
                           TYPE_INDEX_INTMP), k);
      }

      uuid
      addExpr(const Tuple& k, HD* h)
      {
        return nil_uuid();
      }

      private:
      mpz_class m_v;
    };

    class TypeConst : public ExprHD
    {
      public:

      TypeConst(size_t index)
      : ExprHD(0), m_index(index)
      {}

      TaggedValue
      operator()(const Tuple& k)
      {
        return TaggedValue(TypedValue(TypeType(m_index),
                           TYPE_INDEX_TYPE), k);
      }

      private:
      size_t m_index;
    };
  } //namespace ConstHD
} //namespace TransLucid

#endif // CONSTHD_HPP_INCLUDED
