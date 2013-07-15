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
    s.addFunDeclParsed
    (
      Parser::FnDecl
      {
        U"fint" + op,
        //I think clang is broken, this should work without the next line
        std::vector<std::pair<Parser::FnDecl::ArgType, u32string>>
        {
          {Parser::FnDecl::ArgType::CALL_BY_VALUE, U"prec"},
          {Parser::FnDecl::ArgType::CALL_BY_VALUE, U"is_signed"}
        },
        Tree::RegionExpr(
        {
          std::make_tuple
          (Tree::IdentExpr(U"prec"), Region::Containment::IS, mpz_class(size)),
          std::make_tuple
          (Tree::IdentExpr(U"is_signed"), Region::Containment::IS,  is_signed),
        }),
        Tree::Expr(),
        Tree::IdentExpr(name + op)
      }
    );
  }

  template <int size>
  void
  addFloatFunction(System& s, const u32string& name, const u32string& op)
  {
    s.addFunDeclParsed
    (
      Parser::FnDecl
      {
        U"float" + op,
        //I think clang is broken, this should work without the next line
        std::vector<std::pair<Parser::FnDecl::ArgType, u32string>>
        {
          {Parser::FnDecl::ArgType::CALL_BY_VALUE, U"prec"},
        },
        Tree::RegionExpr(
        {
          std::make_tuple(
          Tree::IdentExpr(U"prec"), Region::Containment::IS, mpz_class(size)),
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
  class FixedNumeric
  {
    public:

    void
    init(type_index index, const u32string& name)
    {
      m_index = index;
      m_typename = name;
    }

    template <size_t N, typename F>
    void
    makeEquation
    (
      System& s, 
      const u32string& name, 
      F f,
      std::vector<type_index> type
    )
    {
      //add host function
      BuiltinBaseFunction<N> fun(createFunction(f), type);

      s.addHostFunction(name, &fun, N);
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

    template <typename C>
    UnaryFunction
    createFunction(Constant (C::*f)(const Constant&))
    {
      using std::placeholders::_1;
      return std::bind(std::mem_fn(f), this, _1);
    }

    template <typename C>
    BinaryFunction
    createFunction(Constant (C::*f)
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

    type_index m_index;
    u32string m_typename;
  };

  template <typename T>
  class FixedInteger : private FixedNumeric<T>
  {
    public:

    void
    init(System& s, const u32string& name)
    {
      m_index = s.getTypeIndex(name);
      m_typename = name;
      m_n.init(m_index, m_typename);

      m_n. template 
        makeEquation<1>(s, U"construct_" + name, &FixedNumeric<T>::construct,
          {TYPE_INDEX_USTRING, m_index});
      m_n. template 
        makeEquation<1>(s, U"print_" + name, &FixedNumeric<T>::print,
          {m_index, TYPE_INDEX_USTRING});

      registerArithmeticOperation<2, std::plus>(s, U"_plus");
      registerArithmeticOperation<2, std::minus>(s, U"_minus");
      registerArithmeticOperation<2, std::multiplies>(s, U"_times");
      registerArithmeticOperation<2, std::divides>(s, U"_divide");
      registerArithmeticOperation<2, std::modulus>(s, U"_modulus");

      registerBoolOperation<2, std::equal_to>(s, U"_eq");
      registerBoolOperation<2, std::not_equal_to>(s, U"_ne");
      registerBoolOperation<2, std::less>(s, U"_lt");
      registerBoolOperation<2, std::greater>(s, U"_gt");
      registerBoolOperation<2, std::less_equal>(s, U"_lte");
      registerBoolOperation<2, std::greater_equal>(s, U"_gte");

      addPrinter(s, name, U"print_" + name); 
      addConstructor(s, name, U"construct_" + name);

      addTypeEquation(s, name);
    }

    template <size_t N, template <typename> class F>
    void
    registerBoolOperation
    (
      System& s,
      const u32string& op
    )
    {
      registerFixedOperation<N, F>(s, 
        op, m_index, TYPE_INDEX_BOOL);
    }

    template <size_t N, template <typename> class F>
    void
    registerFixedOperation
    (
      System& s,
      const u32string& op,
      type_index inIndex,
      type_index outIndex
    )
    {
      std::vector<type_index> type;

      for (int i = 0; i != N-1; ++i)
      {
        type.push_back(inIndex);
      }

      type.push_back(outIndex);

      m_n. template makeEquation<N>(s, m_typename + op, 
        NumericOperation<T, F>(inIndex, outIndex), type);

      addIntegerFunction<
        std::is_signed<T>::value,
        sizeof(T) * 8
      > (s, m_typename, op);
    }

    template <size_t N, template <typename> class F>
    void
    registerArithmeticOperation
    (
      System& s,
      const u32string& op
    )
    {
      registerFixedOperation<N, F>(s, op, m_index, m_index);
    }


    type_index m_index;
    u32string m_typename;
    FixedNumeric<T> m_n;
  };

  template <typename T>
  class FixedFloat
  {
    public:

    void
    init(System& s, const u32string& name)
    {
      m_index = s.getTypeIndex(name);
      m_typename = name;
      m_n.init(m_index, m_typename);

      m_n. template 
        makeEquation<1>(s, U"construct_" + name, &FixedNumeric<T>::construct, 
          {TYPE_INDEX_USTRING, m_index});
      m_n. template 
        makeEquation<1>(s, U"print_" + name, &FixedNumeric<T>::print,
          {m_index, TYPE_INDEX_USTRING});

      registerArithmeticOperation<2, std::plus>(s, U"_plus");
      registerArithmeticOperation<2, std::minus>(s, U"_minus");
      registerArithmeticOperation<2, std::multiplies>(s, U"_times");
      registerArithmeticOperation<2, std::divides>(s, U"_divide");
      //registerArithmeticOperation<2, std::modulus>(s, U"_modulus");

      registerBoolOperation<2, std::equal_to>(s, U"_eq");
      registerBoolOperation<2, std::not_equal_to>(s, U"_ne");
      registerBoolOperation<2, std::less>(s, U"_lt");
      registerBoolOperation<2, std::greater>(s, U"_gt");
      registerBoolOperation<2, std::less_equal>(s, U"_lte");
      registerBoolOperation<2, std::greater_equal>(s, U"_gte");

      addPrinter(s, name, U"print_" + name); 
      addConstructor(s, name, U"construct_" + name);

      addTypeEquation(s, name);
    }

    template <size_t N, template <typename> class F>
    void
    registerBoolOperation
    (
      System& s,
      const u32string& op
    )
    {
      registerFixedOperation<N, F>(s, 
        op, m_index, TYPE_INDEX_BOOL);
    }

    template <size_t N, template <typename> class F>
    void
    registerFixedOperation
    (
      System& s,
      const u32string& op,
      type_index inIndex,
      type_index outIndex
    )
    {
      std::vector<type_index> type;

      for (int i = 0; i != N-1; ++i)
      {
        type.push_back(inIndex);
      }

      type.push_back(outIndex);

      m_n. template makeEquation<N>(s, m_typename + op, 
        NumericOperation<T, F>(inIndex, outIndex), type);

      addFloatFunction<
        sizeof(T) * 8
      > (s, m_typename, op);
    }

    template <size_t N, template <typename> class F>
    void
    registerArithmeticOperation
    (
      System& s,
      const u32string& op
    )
    {
      registerFixedOperation<N, F>(s, op, m_index, m_index);
    }

    type_index m_index;
    u32string m_typename;
    FixedNumeric<T> m_n;
  };
}

#endif
