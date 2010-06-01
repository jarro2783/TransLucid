/* The value hyperdatons.
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

#ifndef VALUEHD_HPP_INCLUDED
#define VALUEHD_HPP_INCLUDED

#include <tl/hyperdaton.hpp>
#include <tl/system.hpp>
#include <tl/builtin_types.hpp>
#include <tl/fixed_indexes.hpp>

namespace TransLucid
{
  namespace Hyperdatons
  {
    //TODO: as soon as inheriting constructors is done,
    //      these classes can do
    //using ValueHD::ValueHD;
    class ValueHD : public HD
    {
      public:
      ValueHD(HD* system)
      : m_system(system)
      {}

      protected:
      HD* m_system;
    };

    class BoolHD : public ValueHD
    {
      public:
      static const char32_t* name;

      BoolHD(HD* system)
      : ValueHD(system)
      {}

      TaggedConstant
      operator()(const Tuple& k);
    };

    class IntmpHD : public ValueHD
    {
      public:
      static const char32_t* name;

      IntmpHD(HD* system)
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

  } //namespace Hyperdatons
} //namespace TransLucid

#endif // VALUEHD_HPP_INCLUDED