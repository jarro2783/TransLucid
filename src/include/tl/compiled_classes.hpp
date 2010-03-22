/* Definitions for expressions being compiled to c++.
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

#ifndef COMPILED_CLASSES_HPP_INCLUDED
#define COMPILED_CLASSES_HPP_INCLUDED

#include <tl/types.hpp>

namespace TransLucid
{
  namespace Compiled
  {
    //we probably don't need this
    template <typename Op>
    class GeneralNode
    {
      public:
      GeneralNode(const Op& op)
      : m_op(op)
      {
      }

      TypedValue operator()(const Tuple& t)
      {
        return m_op(t);
      }

      private:
      Op m_op;
    };

    template <bool B>
    class Boolean
    {
      public:
      TypedValue operator()(const Tuple& t)
      {
        return TypedValue(TransLucid::Boolean(B), TYPE_INDEX_BOOL);
      }
    };

    template <bool B>
    Boolean<B> makeBoolean()
    {
      return Boolean<B>();
    }

    template <TransLucid::Special::Value V>
    class Special
    {
      public:

      TypedValue operator()(const Tuple& t)
      {
        return TypedValue(TransLucid::Special(V), TYPE_INDEX_SPECIAL);
      }
    };

    template <TransLucid::Special::Value V>
    Special<V> makeSpecial()
    {
      return Special<V>();
    }

    class Integer
    {
      public:

      template <typename T, T V>
      Integer()
      : m_value(V)
      {
      }

      TypedValue operator()(const Tuple& t)
      {
        return TypedValue(Intmp(m_value), TYPE_INDEX_INTMP);
      }

      private:
      mpz_class m_value;
    };

    template <typename T>
    Integer makeInteger(const T& value)
    {
      return Integer(value);
    }
  }

  template <char32_t C>
  class UChar
  {
    TypedValue operator()(const Tuple& t)
    {
      return TypedValue(Char(C), TYPE_INDEX_UCHAR);
    }
  };

  template <char32_t C>
  UChar<C> makeUChar()
  {
    return UChar<C>();
  }
}

#if 0

#include <string>

#if 0
#include <tl/types.hpp>

namespace TransLucid
{
  typedef <typename T, K N>
  class Constant
  {
    public:

    typedef T RawType;

    RawType
    operator()(Tuple& k)
    {
      return N;
    }

    private:
  };

  typedef <typename T, int32_t N>
  class Constant
  {
    public:

    T
    operator()(Tuple& k)
    {
      return N;
    }

    private:
  };
#endif

extern std::string x;

  template <std::string* N>
  class F
  {
  };

  int
  main()
  {
    //std::string x;
    F<&x> y;
  }

  //int32<3> + int32<4>;;
  //
  //Constant<int32_t, 3>()() + Constant<int32_t, 4>()()

  //Constant<int32_t>(4)() + Constant<int32>(3)();

  //Constant<Glib::ustring,"h">

#if 0
}
#endif

#endif

#endif // COMPILED_CLASSES_HPP_INCLUDED
