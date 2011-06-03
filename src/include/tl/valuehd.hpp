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

#ifndef VALUEWS_HPP_INCLUDED
#define VALUEWS_HPP_INCLUDED

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
    //using ValueWS::ValueWS;
    class ValueWS : public WS
    {
      public:
      ValueWS(WS* system)
      : m_system(system)
      {}

      protected:
      WS* m_system;
    };

    class BoolWS : public ValueWS
    {
      public:
      static const char32_t* name;

      BoolWS(WS* system)
      : ValueWS(system)
      {}

      TaggedConstant
      operator()(const Tuple& k);
    };

    class IntmpWS : public ValueWS
    {
      public:
      static const char32_t* name;

      IntmpWS(WS* system)
      : ValueWS(system)
      {}

      TaggedConstant
      operator()(const Tuple& k);
    };

    class UCharWS : public ValueWS
    {
      public:
      static const char32_t* name;

      UCharWS(WS* system)
      : ValueWS(system)
      {}

      TaggedConstant
      operator()(const Tuple& k);
    };

    class UStringWS : public ValueWS
    {
      public:
      static const char32_t* name;

      UStringWS(WS* system)
      : ValueWS(system)
      {}

      TaggedConstant
      operator()(const Tuple& k);
    };

  } //namespace Hyperdatons
} //namespace TransLucid

#endif // VALUEWS_HPP_INCLUDED
