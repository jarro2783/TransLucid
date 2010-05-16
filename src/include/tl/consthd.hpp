/* The "true" constant hyperdatons.
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
#include <tl/system.hpp>
#include <tl/builtin_types.hpp>
#include <tl/fixed_indexes.hpp>

namespace TransLucid
{
  namespace Hyperdatons
  {
    //TODO: as soon as inheriting constructors is done, these
    //classes can do
    //using ValueHD::ConstantHD;
    class ValueHD : public HD
    {
      public:
      ValueHD(HD* system)
      : m_system(system)
      {}

      protected:
      HD* m_system;
    };

    class Intmp : public ValueHD
    {
      public:
      static const char32_t* name;

      Intmp(HD* system)
      : ValueHD(system)
      {}

      TaggedConstant
      operator()(const Tuple& k);
    };

    class UCharHD : public ValueHD
    {
      public:
      static const char32_t* name;

      UCharHD(HD* system)
      : ValueHD(system)
      {}

      TaggedConstant
      operator()(const Tuple& k);
    };

    class UStringHD : public ValueHD
    {
      public:
      static const char32_t* name;

      UStringHD(HD* system)
      : ValueHD(system)
      {}

      TaggedConstant
      operator()(const Tuple& k);
    };

    //this is not the intmp builder, this returns a constant intmp
    //TODO: This is probably Hyperdatons::Integer
    class IntmpConst : public HD
    {
      public:
      IntmpConst(const mpz_class& v)
      : m_v(v)
      {}

      TaggedConstant
      operator()(const Tuple& k)
      {
        return TaggedConstant(Constant(TransLucid::Intmp(m_v),
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

    //TODO: move this to a ConstantHD
    class TypeHD : public ValueHD
    {
      public:

      TypeHD(size_t index)
      : ValueHD(0), m_index(index)
      {}

      TaggedConstant
      operator()(const Tuple& k)
      {
        return TaggedConstant(Constant(Type(m_index),
                           TYPE_INDEX_TYPE), k);
      }

      private:
      size_t m_index;
    };
  } //namespace Hyperdatons
} //namespace TransLucid

#endif // CONSTHD_HPP_INCLUDED
