/* Fixed number class.
   Copyright (C) 2012 Jarryd Beck

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

#ifndef TL_TYPES_FIXED_NUMBER_HPP_INCLUDED
#define TL_TYPES_FIXED_NUMBER_HPP_INCLUDED

#include <tl/system.hpp>
#include <tl/types/function.hpp>
#include <tl/types/special.hpp>
#include <tl/types/string.hpp>

#include <functional>
#include <sstream>

namespace TransLucid
{
  template <typename T>
  struct read_value
  {
    T 
    operator()(const u32string& s)
    {
      T value;
      std::istringstream is(utf32_to_utf8(s));
      is >> value;

      return value;
    }
  };

  template <>
  struct read_value<int8_t>
  {
    int8_t
    operator()(const u32string& s)
    {
      return read_value<int16_t>()(s);
    }
  };

  template <>
  struct read_value<uint8_t>
  {
    uint8_t
    operator()(const u32string& s)
    {
      return read_value<uint16_t>()(s);
    }
  };

  template <typename T>
  struct write_value
  {
    std::string
    operator()(T value)
    {
      std::ostringstream os;
      os << value;
      return os.str();
    }
  };

  template <>
  struct write_value<int8_t>
  {
    std::string
    operator()(int8_t value)
    {
      return write_value<int16_t>()(value);
    }
  };

  template <>
  struct write_value<uint8_t>
  {
    std::string
    operator()(uint8_t value)
    {
      return write_value<uint16_t>()(value);
    }
  };

  template <typename T>
  class FixedInteger
  {
    public:

    void
    init(System& s, const u32string& name)
    {
      m_index = s.getTypeIndex(name);
      m_typename = name;

      makeEquation(s, U"construct_" + name, &FixedInteger::construct);
      makeEquation(s, U"print_" + name, &FixedInteger::print);

      addPrinter(s, name, U"print_" + name); 
      addConstructor(s, name, U"construct_" + name);

      addTypeEquation(s, name);
    }

    Constant
    construct(const Constant& c)
    {
      T value;

      if (c.index() != TYPE_INDEX_USTRING)
      {
        return Types::Special::create(SP_UNDEF);
      }

      const u32string& s = Types::String::get(c);

      value = read_value<T>()(s);

      return Constant(static_cast<T>(value), m_index);
    }

    Constant
    print(const Constant& c)
    {
      T value = get_constant<T>(c);

      auto str = write_value<T>()(value);
      return Types::String::create
        (u32string(str.begin(), str.end()));
    }

    private:

    typedef 
    std::function<Constant(const Constant&)>
    UnaryFunction;

    template <typename Ret, typename... Args>
    void
    makeEquation
    (
      System& s, 
      const u32string& name, 
      Ret (FixedInteger<T>::*f)(Args...)
    )
    {
      std::unique_ptr<BuiltinBaseFunction<sizeof...(Args)>> 
        fn(new BuiltinBaseFunction<sizeof...(Args)>
        (createUnaryFunction(f))
      );
      std::unique_ptr<BangAbstractionWS> ws(new BangAbstractionWS(fn.get()));
      fn.release();
      s.addEquation(name, ws.get());
      ws.release();
    }

    UnaryFunction
    createUnaryFunction(Constant (FixedInteger<T>::*f)(const Constant&))
    {
      using std::placeholders::_1;
      return std::bind(std::mem_fun(f), this, _1);
    }

    type_index m_index;
    u32string m_typename;
  };
}

#endif
