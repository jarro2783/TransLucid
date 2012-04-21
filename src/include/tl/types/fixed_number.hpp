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

  template <bool is_signed, int size>
  void
  addNumericFunction(System& s, const u32string& name, const u32string& op)
  {
    s.addFunction
    (
      Parser::FnDecl
      {
        U"numeric" + op,
        {
          {Parser::FnDecl::ArgType::CALL_BY_VALUE, U"prec"},
          {Parser::FnDecl::ArgType::CALL_BY_VALUE, U"is_signed"}
        },
        Tree::TupleExpr(
        {
          {Tree::IdentExpr(U"prec"), mpz_class(size)},
          {Tree::IdentExpr(U"is_signed"), is_signed},
        }),
        Tree::Expr(),
        Tree::IdentExpr(name + op)
      }
    );
  }

  template <typename T>
  class FixedInteger
  {
    struct yes {};
    struct no {};
    public:

    void
    init(System& s, const u32string& name)
    {
      m_index = s.getTypeIndex(name);
      m_typename = name;

      makeEquation(s, U"construct_" + name, &FixedInteger::construct);
      makeEquation(s, U"print_" + name, &FixedInteger::print);
      //makeEquation(s, name + U"_plus", &FixedInteger::plus);

      registerOperation(s, name, U"_plus", &FixedInteger::plus);
      registerOperation(s, name, U"_minus", &FixedInteger::minus);
      registerOperation(s, name, U"_times", &FixedInteger::times);
      registerOperation(s, name, U"_divide", &FixedInteger::divide);

      registerModulus(s, name, 
        typename std::conditional<
          std::is_floating_point<T>::value, no, yes>::type());

      registerOperation(s, name, U"_eq", &FixedInteger::eq);
      registerOperation(s, name, U"_ne", &FixedInteger::ne);
      registerOperation(s, name, U"_lt", &FixedInteger::lt);
      registerOperation(s, name, U"_gt", &FixedInteger::gt);
      registerOperation(s, name, U"_leq", &FixedInteger::leq);
      registerOperation(s, name, U"_geq", &FixedInteger::geq);

      addPrinter(s, name, U"print_" + name); 
      addConstructor(s, name, U"construct_" + name);

      addTypeEquation(s, name);

      //add the function for each op
    }

    Constant
    construct(const Constant& c)
    {
      T value;

      if (c.index() != TYPE_INDEX_USTRING)
      {
        return Types::Special::create(SP_TYPEERROR);
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

    Constant
    plus(const Constant& lhs, const Constant& rhs)
    {
      if (lhs.index() != m_index || rhs.index() != m_index)
      {
        return Types::Special::create(SP_TYPEERROR);
      }

      return Constant(get_constant<T>(lhs) + get_constant<T>(rhs), m_index);
    }

    Constant
    minus(const Constant& lhs, const Constant& rhs)
    {
      if (lhs.index() != m_index || rhs.index() != m_index)
      {
        return Types::Special::create(SP_TYPEERROR);
      }

      return Constant(get_constant<T>(lhs) - get_constant<T>(rhs), m_index);
    }

    Constant
    times(const Constant& lhs, const Constant& rhs)
    {
      if (lhs.index() != m_index || rhs.index() != m_index)
      {
        return Types::Special::create(SP_TYPEERROR);
      }

      return Constant(get_constant<T>(lhs) * get_constant<T>(rhs), m_index);
    }

    Constant
    divide(const Constant& lhs, const Constant& rhs)
    {
      if (lhs.index() != m_index || rhs.index() != m_index)
      {
        return Types::Special::create(SP_TYPEERROR);
      }

      if (get_constant<T>(rhs) == 0)
      {
        return Types::Special::create(SP_TYPEERROR);
      }

      return Constant(get_constant<T>(lhs) / get_constant<T>(rhs), m_index);
    }

    Constant
    modulus(const Constant& lhs, const Constant& rhs)
    {
      if (lhs.index() != m_index || rhs.index() != m_index)
      {
        return Types::Special::create(SP_TYPEERROR);
      }

      if (get_constant<T>(rhs) == 0)
      {
        return Types::Special::create(SP_TYPEERROR);
      }

      return Constant(get_constant<T>(lhs) % get_constant<T>(rhs), m_index);
    }

    Constant
    eq(const Constant& lhs, const Constant& rhs)
    {
      if (lhs.index() != m_index || rhs.index() != m_index)
      {
        return Types::Special::create(SP_TYPEERROR);
      }

      return Constant(get_constant<T>(lhs) == get_constant<T>(rhs), 
        TYPE_INDEX_BOOL);
    }

    Constant
    ne(const Constant& lhs, const Constant& rhs)
    {
      if (lhs.index() != m_index || rhs.index() != m_index)
      {
        return Types::Special::create(SP_TYPEERROR);
      }

      return Constant(get_constant<T>(lhs) != get_constant<T>(rhs), 
        TYPE_INDEX_BOOL);
    }

    Constant
    lt(const Constant& lhs, const Constant& rhs)
    {
      if (lhs.index() != m_index || rhs.index() != m_index)
      {
        return Types::Special::create(SP_TYPEERROR);
      }

      return Constant(get_constant<T>(lhs) < get_constant<T>(rhs), 
        TYPE_INDEX_BOOL);
    }

    Constant
    gt(const Constant& lhs, const Constant& rhs)
    {
      if (lhs.index() != m_index || rhs.index() != m_index)
      {
        return Types::Special::create(SP_TYPEERROR);
      }

      return Constant(get_constant<T>(lhs) > get_constant<T>(rhs), 
        TYPE_INDEX_BOOL);
    }

    Constant
    leq(const Constant& lhs, const Constant& rhs)
    {
      if (lhs.index() != m_index || rhs.index() != m_index)
      {
        return Types::Special::create(SP_TYPEERROR);
      }

      return Constant(get_constant<T>(lhs) <= get_constant<T>(rhs), 
        TYPE_INDEX_BOOL);
    }

    Constant
    geq(const Constant& lhs, const Constant& rhs)
    {
      if (lhs.index() != m_index || rhs.index() != m_index)
      {
        return Types::Special::create(SP_TYPEERROR);
      }

      return Constant(get_constant<T>(lhs) >= get_constant<T>(rhs), 
        TYPE_INDEX_BOOL);
    }

    private:

    typedef 
    std::function<Constant(const Constant&)>
    UnaryFunction;

    typedef 
    std::function<Constant(const Constant&, const Constant&)>
    BinaryFunction;

    void
    registerModulus(System& s, const u32string& name, no n)
    {
    }

    void
    registerModulus(System& s, const u32string& name, yes y)
    {
      registerOperation(s, name, U"_modulus", &FixedInteger::modulus);
    }

    template <typename Fun>
    void
    registerOperation
    (
      System& s,
      const u32string& name,
      const u32string& op,
      Fun f
    )
    {
      makeEquation(s, name + op, f);

      addNumericFunction<
        std::is_signed<T>::value,
        sizeof(T) * 8
      > (s, name, op);
    }

    UnaryFunction
    createFunction(Constant (FixedInteger<T>::*f)(const Constant&))
    {
      using std::placeholders::_1;
      return std::bind(std::mem_fn(f), this, _1);
    }

    BinaryFunction
    createFunction(Constant (FixedInteger<T>::*f)
      (const Constant&, const Constant&))
    {
      using std::placeholders::_1;
      using std::placeholders::_2;
      return std::bind(std::mem_fn(f), this, _1, _2);
    }

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
          (createFunction(f))
      );
      std::unique_ptr<BangAbstractionWS> ws(new BangAbstractionWS(fn.get()));
      fn.release();
      s.addEquation(name, ws.get());
      ws.release();
    }

    type_index m_index;
    u32string m_typename;
  };
}

#endif
