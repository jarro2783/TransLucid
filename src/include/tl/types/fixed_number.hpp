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
  addIntegerFunction(System& s, const u32string& name, const u32string& op)
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

  template <typename T, template <typename> class Op>
  class NumericOperation
  {
    public:
    NumericOperation(type_index input, type_index output)
    : m_input(input), m_output(output)
    {
    }

    Constant
    operator()(const Constant& lhs, const Constant& rhs)
    {
      if (lhs.index() != m_input || rhs.index() != m_input)
      {
        return Types::Special::create(SP_TYPEERROR);
      }
      return Constant(m_op(get_constant<T>(lhs), get_constant<T>(rhs)), 
                      m_output);
    }

    private:
    Op<T> m_op;
    type_index m_input;
    type_index m_output;
  };

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

      makeEquation<1>(s, U"construct_" + name, &FixedInteger::construct);
      makeEquation<1>(s, U"print_" + name, &FixedInteger::print);

      registerArithmeticOperation<2, std::plus>(s, name, U"_plus");
      registerArithmeticOperation<2, std::minus>(s, name, U"_minus");
      registerArithmeticOperation<2, std::multiplies>(s, name, U"_times");
      registerArithmeticOperation<2, std::divides>(s, name, U"_divide");
      registerArithmeticOperation<2, std::modulus>(s, name, U"_modulus");

      registerBoolOperation<2, std::equal_to>(s, name, U"_eq");
      registerBoolOperation<2, std::not_equal_to>(s, name, U"_ne");
      registerBoolOperation<2, std::less>(s, name, U"_lt");
      registerBoolOperation<2, std::greater>(s, name, U"_gt");
      registerBoolOperation<2, std::less_equal>(s, name, U"_lte");
      registerBoolOperation<2, std::greater_equal>(s, name, U"_gte");

      addPrinter(s, name, U"print_" + name); 
      addConstructor(s, name, U"construct_" + name);

      addTypeEquation(s, name);
    }

    template <size_t N, template <typename> class F>
    void
    registerBoolOperation
    (
      System& s,
      const u32string& name,
      const u32string& op
    )
    {
      registerFixedOperation<N, F>(s, name, op, m_index, TYPE_INDEX_BOOL);
    }

    template <size_t N, template <typename> class F>
    void
    registerFixedOperation
    (
      System& s,
      const u32string& name,
      const u32string& op,
      type_index inIndex,
      type_index outIndex
    )
    {
      makeEquation<N>(s, name + op, NumericOperation<T, F>(inIndex, outIndex));

      addIntegerFunction<
        std::is_signed<T>::value,
        sizeof(T) * 8
      > (s, name, op);
    }

    template <size_t N, template <typename> class F>
    void
    registerArithmeticOperation
    (
      System& s,
      const u32string& name,
      const u32string& op
    )
    {
      registerFixedOperation<N, F>(s, name, op, m_index, m_index);
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

    private:

    typedef 
    std::function<Constant(const Constant&)>
    UnaryFunction;

    typedef 
    std::function<Constant(const Constant&, const Constant&)>
    BinaryFunction;

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

    template <typename F>
    F
    createFunction(F f)
    {
      return f;
    }

    template <size_t N, typename F>
    void
    makeEquation
    (
      System& s, 
      const u32string& name, 
      F f
    )
    {
      std::unique_ptr<BuiltinBaseFunction<N>> 
        fn(new BuiltinBaseFunction<N>
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
