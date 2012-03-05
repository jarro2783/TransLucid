/* Built-in types.
   Copyright (C) 2009, 2010, 2011 Jarryd Beck and John Plaice

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

/** @file builtin_types.cpp
 * Builtin type definitions.
 */

#include <sstream>

#include <unicode/uchar.h>

#include <tl/ast.hpp>
#include <tl/builtin_types.hpp>
#include "tl/builtin_ops.hpp"
#include <tl/equation.hpp>
//#include <tl/hyperdatons/filehd.hpp>
#include <tl/internal_strings.hpp>
#include <tl/output.hpp>
#include <tl/system.hpp>
#include <tl/types.hpp>
#include <tl/types/boolean.hpp>
#include <tl/types/char.hpp>
#include <tl/types/demand.hpp>
#include <tl/types/function.hpp>
#include <tl/types/hyperdatons.hpp>
#include <tl/types/intmp.hpp>
#include <tl/types/range.hpp>
#include <tl/types/string.hpp>
#include <tl/types/tuple.hpp>
#include <tl/types/type.hpp>
#include <tl/types/uuid.hpp>
#include <tl/types/workshop.hpp>
#include <tl/types_util.hpp>
#include <tl/utility.hpp>

#include <gmpxx.h>

namespace TransLucid
{

  Constant
  print_error_value(const Constant& arg);

  namespace
  {
    using namespace TransLucid::BuiltinOps;

    BuiltinBaseFunction<1> construct_integer{
      static_cast<Constant (*)(const Constant&)>(&Types::Intmp::create)
    };

    BuiltinBaseFunction<1> construct_special{
      static_cast<Constant (*)(const Constant&)>(&Types::Special::create)
    };

    BuiltinBaseFunction<1> construct_uuid {
      static_cast<Constant (*)(const Constant&)>(&Types::UUID::create)
    };

    BuiltinBaseFunction<1> print_intmp{&Types::Intmp::print};
    BuiltinBaseFunction<1> print_uchar{&Types::UChar::print};
    //BuiltinBaseFunction<1> print_special_base{&Types::Special::print};
    BuiltinBaseFunction<1> print_bool{&Types::Boolean::print};
    BuiltinBaseFunction<1> print_range{&Types::Range::print};
    BuiltinBaseFunction<1> print_typetype{&Types::Type::print};
    BuiltinBaseFunction<1> print_error{&print_error_value};
    BuiltinBaseFunction<1> print_uuid{&Types::UUID::print};

    BuiltinBaseFunction<2> integer_plus{&mpz_plus};
    BuiltinBaseFunction<2> integer_minus{&mpz_minus};
    BuiltinBaseFunction<2> integer_times{&mpz_times};
    BuiltinBaseFunction<2> integer_divide{&mpz_divide};
    BuiltinBaseFunction<2> integer_modulus{&mpz_modulus};
    BuiltinBaseFunction<2> integer_lte{&mpz_lte};
    BuiltinBaseFunction<2> integer_lt{&mpz_lt};
    BuiltinBaseFunction<2> integer_gte{&mpz_gte};
    BuiltinBaseFunction<2> integer_gt{&mpz_gt};
    BuiltinBaseFunction<2> integer_eq{&mpz_eq};
    BuiltinBaseFunction<2> integer_ne{&mpz_ne};

    BuiltinBaseFunction<2> ustring_plus_fn{&ustring_plus};

    BuiltinBaseFunction<2> boolean_eq{&bool_eq};

    BuiltinBaseFunction<2> range_create{
      static_cast<Constant (*)(const Constant&, const Constant&)>
        (&Types::Range::create)
    };

    BuiltinBaseFunction<1> range_create_inf{
      [] (const Constant& lhs) -> Constant
      {
        const mpz_class* lhsp = &get_constant_pointer<mpz_class>(lhs);
        return Types::Range::create(TransLucid::Range(lhsp, nullptr));
      }
    };

    BuiltinBaseFunction<1> range_create_neginf{
      [] (const Constant& rhs) -> Constant
      {
        const mpz_class* rhsp = &get_constant_pointer<mpz_class>(rhs);
        return Types::Range::create(TransLucid::Range(nullptr, rhsp));
      }
    };

    BuiltinBaseFunction<0> range_create_infinity{
      [] () -> Constant
      {
        return Types::Range::create(TransLucid::Range(nullptr, nullptr));
      }
    };

    BuiltinBaseFunction<1> icu_is_printable{
      [] (const Constant& c) -> Constant
      {
        if (c.index() != TYPE_INDEX_UCHAR)
        {
          return Types::Special::create(Special::SP_TYPEERROR);
        }
        else
        {
          return Types::Boolean::create(u_isprint(get_constant<char32_t>(c)));
        }
      }
    };

    BuiltinBaseFunction<1> uchar_code_point{
      [] (const Constant& c) -> Constant
      {
        if (c.index() != TYPE_INDEX_UCHAR)
        {
          return Types::Special::create(Special::SP_TYPEERROR);
        }
        else
        {
          return Types::Intmp::create(get_constant<char32_t>(c));
        }
      }
    };

    BuiltinBaseFunction<2> string_at_base{
      [] (const Constant& s, const Constant& at) -> Constant
      {
        if (s.index() != TYPE_INDEX_USTRING || at.index() != TYPE_INDEX_INTMP)
        {
          return Types::Special::create(Special::SP_TYPEERROR);
        }
        else
        {
          const u32string& string = get_constant_pointer<u32string>(s);
          const mpz_class& index = get_constant_pointer<mpz_class>(at);

          if (index < string.length())
          {
            return Types::UChar::create(string.at(index.get_si()));
          }
          else
          {
            return Types::Special::create(Special::SP_UNDEF);
          }
        }
      }
    };

    BuiltinBaseFunction<3> substring_base{
      [] (const Constant& s, const Constant& start, const Constant& length)
        -> Constant
      {
        if (s.index() != TYPE_INDEX_USTRING || 
            start.index() != TYPE_INDEX_INTMP ||
            length.index() != TYPE_INDEX_INTMP)
        {
          return Types::Special::create(Special::SP_TYPEERROR);
        }
        else
        {
          const u32string& string = get_constant_pointer<u32string>(s);
          const mpz_class& startz = get_constant_pointer<mpz_class>(start);
          const mpz_class& lengthz = get_constant_pointer<mpz_class>(length);

          return Types::String::create(string.substr(
            startz.get_si(), lengthz.get_si()));
        }
      }
    };

    BuiltinBaseFunction<2> substring_toend_base{
      [] (const Constant& s, const Constant& start)
        -> Constant
      {
        if (s.index() != TYPE_INDEX_USTRING || 
            start.index() != TYPE_INDEX_INTMP)
        {
          return Types::Special::create(Special::SP_TYPEERROR);
        }
        else
        {
          const u32string& string = get_constant_pointer<u32string>(s);
          const mpz_class& number = get_constant_pointer<mpz_class>(start);

          return Types::String::create(string.substr(number.get_si(), 
                   u32string::npos));
        }
      }
    };

    Constant code_point_n(const Constant& c, int numDigits)
    {
      u32string text(numDigits, U'0');
      if (c.index() != TYPE_INDEX_UCHAR)
      {
        return Types::Special::create(Special::SP_TYPEERROR);
      }
      else
      {
        char32_t val = get_constant<char32_t>(c);
        while (numDigits != 0)
        {
          int current = val % 16;

          if (current >= 0 && current <= 9)
          {
            text[numDigits-1] = static_cast<char32_t>(current + '0');
          }
          else
          {
            text[numDigits-1] = static_cast<char32_t>(current + 'A' - 10);
          }
          val = val / 16;
          --numDigits;
        }
      }
      
      return Types::String::create(text);
    }

    BuiltinBaseFunction<1> code_point_4{
      [] (const Constant& c) -> Constant
      {
        return code_point_n(c, 4);
      }
    };

    BuiltinBaseFunction<1> code_point_8{
      [] (const Constant& c) -> Constant
      {
        return code_point_n(c, 8);
      }
    };

    struct BuiltinFunction
    {
      const char32_t* op_name;
      BaseFunctionType* fn;
    };

    constexpr BuiltinFunction fn_table[] =
    {
      {U"int_plus", &integer_plus},
      {U"int_minus", &integer_minus},
      {U"int_times", &integer_times},
      {U"int_divide", &integer_divide},
      {U"int_modulus", &integer_modulus},
      {U"int_lte", &integer_lte},
      {U"int_lt", &integer_lt},
      {U"int_gte", &integer_gte},
      {U"int_gt", &integer_gt},
      {U"int_eq", &integer_eq},
      {U"int_ne", &integer_ne},
      {U"bool_eq", &boolean_eq},
      {U"ustring_plus", &ustring_plus_fn},
      {U"make_range", &range_create},
      {U"make_range_infty", &range_create_inf},
      {U"make_range_neginfty", &range_create_neginf},
      {U"make_range_infinite", &range_create_infinity},
      {U"construct_intmp", &construct_integer},
      {U"construct_special", &construct_special},
      {U"construct_uuid", &construct_uuid},
      {U"is_printable", &icu_is_printable},
      {U"string_at_base", &string_at_base},
      {U"substring_base", &substring_base},
      {U"substring_toend_base", &substring_toend_base},
      {U"code_point", &uchar_code_point},
      {U"code_point_4", &code_point_4},
      {U"code_point_8", &code_point_8},
      {U"print_intmp", &print_intmp},
      {U"print_uchar", &print_uchar},
      //{U"print_special", &print_special},
      {U"print_bool", &print_bool},
      {U"print_range", &print_range},
      {U"print_typetype", &print_typetype},
      {U"print_error", &print_error},
      {U"print_uuid", &print_uuid},
    };
  }

  Constant
  print_error_value(const Constant& arg)
  {
    return Types::String::create(U"internal compiler error");
  }
}

namespace std
{
  template<>
  struct hash<mpz_class>
  {
    size_t
    operator()(const mpz_class& v) const
    {
      return std::hash<std::string>()(v.get_str());
    }
  };
}

namespace TransLucid
{
  void
  init_file_hds(System& s);

  namespace 
  {
    TypeFunctions string_type_functions =
      {
        &Types::String::equality,
        &Types::String::hash,
        &delete_ptr<u32string>
      };

    TypeFunctions base_function_type_functions =
      {
        &Types::BaseFunction::equality,
        &Types::BaseFunction::hash,
        &delete_ptr<BaseFunctionType>
      };

    TypeFunctions value_function_type_functions =
      {
        &Types::ValueFunction::equality,
        &Types::ValueFunction::hash,
        &delete_ptr<ValueFunctionType>
      };

    TypeFunctions name_function_type_functions =
      {
        &Types::NameFunction::equality,
        &Types::NameFunction::hash,
        &delete_ptr<NameFunctionType>
      };

    TypeFunctions range_type_functions =
      {
        &Types::Range::equality,
        &Types::Range::hash,
        &delete_ptr<Range>
      };

    TypeFunctions tuple_type_functions =
      {
        &Types::Tuple::equality,
        &Types::Tuple::hash,
        &delete_ptr<Tuple>
      };

    TypeFunctions intmp_type_functions = 
      {
        &Types::Intmp::equality,
        &Types::Intmp::hash,
        &delete_ptr<mpz_class>
      };

    TypeFunctions hyperdaton_type_functions =
      {
        &Types::Hyperdatons::equality,
        &Types::Hyperdatons::hash,
        &delete_ptr<HD>
      };

    TypeFunctions workshop_type_functions =
      {
        &Types::Workshop::equality,
        &Types::Workshop::hash,
        &delete_ptr<WorkshopType>
      };

    TypeFunctions demand_type_functions =
      {
        &Types::Demand::equality,
        &Types::Demand::hash,
        &delete_ptr<DemandType>
      };

    //copied and pasted from types.hpp, this should match the strings below
    #if 0
    SP_ERROR, /**<Error value. Should never have this value, having a special
    of this value means an error occured somewhere.*/
    SP_ACCESS, /**<Access error. Something requested could not be accessed.*/
    SP_TYPEERROR,
    SP_DIMENSION,
    SP_ARITH,
    SP_UNDEF,
    SP_CONST,
    SP_MULTIDEF,
    SP_LOOP,
    SPECIAL_LAST //the number of specials, not an actual special value
    #endif

    u32string special_names[SPECIAL_LAST] = 
    {
      U"error",
      U"access",
      U"typeerror",
      U"dim",
      U"arith",
      U"undef",
      U"const",
      U"multidef",
      U"loop"
      //SPECIAL_LAST is not an actual value
    }
    ;

    std::unordered_map<u32string, TransLucid::Special> special_map
    {
      {U"sperror", SP_ERROR},
      {U"spaccess", SP_ACCESS},
      {U"sptypeerror", SP_TYPEERROR},
      {U"sparith", SP_ARITH},
      {U"spundef", SP_UNDEF},
      {U"spconst", SP_CONST},
      {U"spmultidef", SP_MULTIDEF},
      {U"sploop", SP_LOOP}
    };

  }

  namespace BuiltinOps
  {
    Constant
    mpz_plus(const Constant& a, const Constant& b)
    {
      return Types::Intmp::create(get_constant_pointer<mpz_class>(a) +
        get_constant_pointer<mpz_class>(b))
      ;
    }

    Constant
    mpz_minus(const Constant& a, const Constant& b)
    {
      return Types::Intmp::create(get_constant_pointer<mpz_class>(a) -
        get_constant_pointer<mpz_class>(b))
      ;
    }

    Constant
    mpz_times(const Constant& a, const Constant& b)
    {
      return Types::Intmp::create(get_constant_pointer<mpz_class>(a) *
        get_constant_pointer<mpz_class>(b))
      ;
    }

    Constant
    mpz_divide(const Constant& a, const Constant& b)
    {
      mpz_class bval = get_constant_pointer<mpz_class>(b);
      if (bval == 0)
      {
        return Types::Special::create(Special::SP_ARITH);
      }
      else
      {
        return Types::Intmp::create(get_constant_pointer<mpz_class>(a) /
          bval);
      }
      ;
    }

    Constant
    mpz_modulus(const Constant& a, const Constant& b)
    {
      mpz_class bval = get_constant_pointer<mpz_class>(b);
      if (bval == 0)
      {
        return Types::Special::create(Special::SP_ARITH);
      }
      else
      {
        return Types::Intmp::create(get_constant_pointer<mpz_class>(a) %
          bval);
      }
      ;
    }

    Constant
    mpz_lte(const Constant& a, const Constant& b)
    {
      return Types::Boolean::create(get_constant_pointer<mpz_class>(a) <=
        get_constant_pointer<mpz_class>(b))
      ;
    }

    Constant
    mpz_lt(const Constant& a, const Constant& b)
    {
      return Types::Boolean::create(get_constant_pointer<mpz_class>(a) <
        get_constant_pointer<mpz_class>(b))
      ;
    }

    Constant
    mpz_gte(const Constant& a, const Constant& b)
    {
      return Types::Boolean::create(get_constant_pointer<mpz_class>(a) >=
        get_constant_pointer<mpz_class>(b))
      ;
    }

    Constant
    mpz_gt(const Constant& a, const Constant& b)
    {
      return Types::Boolean::create(get_constant_pointer<mpz_class>(a) >
        get_constant_pointer<mpz_class>(b))
      ;
    }

    Constant
    mpz_eq(const Constant& a, const Constant& b)
    {
      return Types::Boolean::create(get_constant_pointer<mpz_class>(a) ==
        get_constant_pointer<mpz_class>(b))
      ;
    }

    Constant
    mpz_ne(const Constant& a, const Constant& b)
    {
      return Types::Boolean::create(get_constant_pointer<mpz_class>(a) !=
        get_constant_pointer<mpz_class>(b))
      ;
    }

    Constant
    bool_eq(const Constant& a, const Constant& b)
    {
      return Types::Boolean::create(get_constant<bool>(a) ==
        get_constant<bool>(b))
      ;
    }

    Constant
    bool_ne(const Constant& a, const Constant& b)
    {
      return Types::Boolean::create(get_constant<bool>(a) !=
        get_constant<bool>(b))
      ;
    }

    Constant
    bool_and(const Constant& a, const Constant& b)
    {
      return Types::Boolean::create(get_constant<bool>(a) &&
        get_constant<bool>(b))
      ;
    }

    Constant
    bool_or(const Constant& a, const Constant& b)
    {
      return Types::Boolean::create(get_constant<bool>(a) ||
        get_constant<bool>(b))
      ;
    }

    Constant
    ustring_eq(const Constant& a, const Constant& b)
    {
      return Types::Boolean::create(get_constant_pointer<u32string>(a) ==
        get_constant_pointer<u32string>(b))
      ;
    }

    Constant
    ustring_ne(const Constant& a, const Constant& b)
    {
      return Types::Boolean::create(get_constant_pointer<u32string>(a) !=
        get_constant_pointer<u32string>(b))
      ;
    }

    Constant
    ustring_plus(const Constant& a, const Constant& b)
    {
      return Types::String::create(get_constant_pointer<u32string>(a) +
        get_constant_pointer<u32string>(b))
      ;
    }
  }

  namespace detail
  {
    template <>
    struct clone<u32string>
    {
      u32string*
      operator()(const u32string& v)
      {
        return new u32string(v);
      }
    };

    template <>
    struct clone<BaseFunctionType>
    {
      BaseFunctionType*
      operator()(const BaseFunctionType& b)
      {
        return b.clone();
      }
    };

    template <>
    struct clone<NameFunctionType>
    {
      NameFunctionType*
      operator()(const NameFunctionType& n)
      {
        return n.clone();
      }
    };

    template <>
    struct clone<ValueFunctionType>
    {
      ValueFunctionType*
      operator()(const ValueFunctionType& v)
      {
        return v.clone();
      }
    };

    template <>
    struct clone<Range>
    {
      Range*
      operator()(const Range& r)
      {
        return new Range(r);
      }
    };

    template <>
    struct clone<mpz_class>
    {
      mpz_class*
      operator()(const mpz_class& i)
      {
        return new mpz_class(i);
      }
    };

    template <>
    struct clone<Tuple>
    {
      Tuple*
      operator()(const Tuple& t)
      {
        return new Tuple(t);
      }
    };

    template <>
    struct clone<DemandType>
    {
      DemandType*
      operator()(const DemandType& d)
      {
        return new DemandType(d);
      }
    };
  }

  namespace Types
  {
    namespace String
    {
      Constant
      create(const u32string& s)
      {
        return make_constant_pointer
          (s, &string_type_functions, TYPE_INDEX_USTRING);
        //std::unique_ptr<u32string> value(new u32string(s));
        //ConstantPointerValue* p = 
        //  new ConstantPointerValue(&string_type_functions, value.get());
        //value.release();
        //return Constant(p, TYPE_INDEX_USTRING);
      }

      const u32string&
      get(const Constant& s)
      {
        return get_constant_pointer<u32string>(s);
      }

      size_t
      hash(const Constant& c)
      {
        return std::hash<u32string>()(get(c));
      }

      bool
      equality(const Constant& lhs, const Constant& rhs)
      {
        //same pointer means same string
        return lhs.data.ptr == rhs.data.ptr
          || get(lhs) == get(rhs);
      }
    }

    namespace Boolean
    {
      Constant
      create(bool v)
      {
        return Constant(v, TYPE_INDEX_BOOL);
      }

      Constant
      print(const Constant& c)
      {
        bool b = get_constant<bool>(c);
        if (b)
        {
          return String::create(U"true");
        }
        else
        {
          return String::create(U"false");
        }
      }
    }

    namespace Type
    {
      Constant
      create(type_index t)
      {
        return Constant(t, TYPE_INDEX_TYPE);
      }

      Constant
      print(const Constant& v)
      {
        std::ostringstream os;
        os << get_constant<type_index>(v);

        std::string s = os.str();
        return String::create(u32string(s.begin(), s.end()));
      }
    }
    
    namespace BaseFunction
    {
      Constant
      create(const BaseFunctionType& f)
      {
        return make_constant_pointer
          (f, &base_function_type_functions, TYPE_INDEX_BASE_FUNCTION);
      }

      const BaseFunctionType&
      get(const Constant& c)
      {
        return get_constant_pointer<BaseFunctionType>(c);
      }

      size_t
      hash(const Constant& c)
      {
        return get(c).hash();
      }

      bool
      equality(const Constant& lhs, const Constant& rhs)
      {
        return lhs.data.ptr->data == rhs.data.ptr->data;
      }
    }

    namespace ValueFunction
    {
      Constant
      create(const ValueFunctionType& f)
      {
        return make_constant_pointer
          (f, &value_function_type_functions, TYPE_INDEX_VALUE_FUNCTION);
      }

      const ValueFunctionType&
      get(const Constant& c)
      {
        return get_constant_pointer<ValueFunctionType>(c);
      }

      size_t
      hash(const Constant& c)
      {
        return get(c).hash();
      }

      bool
      equality(const Constant& lhs, const Constant& rhs)
      {
        return lhs.data.ptr->data == rhs.data.ptr->data;
      }
    }

    namespace NameFunction
    {
      Constant
      create(const NameFunctionType& f)
      {
        return make_constant_pointer
          (f, &name_function_type_functions, TYPE_INDEX_NAME_FUNCTION);
      }

      const NameFunctionType&
      get(const Constant& c)
      {
        return get_constant_pointer<NameFunctionType>(c);
      }

      size_t
      hash(const Constant& c)
      {
        return get(c).hash();
      }

      bool
      equality(const Constant& lhs, const Constant& rhs)
      {
        return lhs.data.ptr->data == rhs.data.ptr->data;
      }
    }

    namespace Special
    {
      Constant
      create(TransLucid::Special s)
      {
        return Constant(s, TYPE_INDEX_SPECIAL);
      }

      Constant
      create(const Constant& c)
      {
        const u32string& s = get_constant_pointer<u32string>(c);
        auto iter = special_map.find(s);

        if (iter != special_map.end())
        {
          return create(iter->second);
        }
        else
        {
          return create(SP_CONST);
        }
      }

      Constant
      print(const Constant& c)
      {
        auto s = get_constant<TransLucid::Special>(c);

        return String::create(U"sp" + special_names[s]);
      }
    }

    namespace Range
    {
      Constant
      create(const Constant& lhs, const Constant& rhs)
      {
        const mpz_class* lhsp = &get_constant_pointer<mpz_class>(lhs);
        const mpz_class* rhsp = &get_constant_pointer<mpz_class>(rhs);

        return Range::create(TransLucid::Range(lhsp, rhsp));
      }

      Constant
      create(const TransLucid::Range& r)
      {
        return make_constant_pointer
          (r, &range_type_functions, TYPE_INDEX_RANGE);
      }

      const TransLucid::Range&
      get(const Constant& r)
      {
        return get_constant_pointer<TransLucid::Range>(r);
      }

      bool 
      equality(const Constant& lhs, const Constant& rhs)
      {
        return get(lhs) == get(rhs);
      }

      size_t
      hash(const Constant& c)
      {
        return get(c).hash();
      }

      Constant
      print(const Constant& c)
      {
        return String::create(U"range");
      }
    }

    namespace Tuple
    {
      Constant
      create(const TransLucid::Tuple& t)
      {
        return make_constant_pointer
          (t, &tuple_type_functions, TYPE_INDEX_TUPLE);
      }

      const TransLucid::Tuple&
      get(const Constant& t)
      {
        return get_constant_pointer<TransLucid::Tuple>(t);
      }

      bool 
      equality(const Constant& lhs, const Constant& rhs)
      {
        return get(lhs) == get(rhs);
      }

      size_t
      hash(const Constant& c)
      {
        return get(c).hash();
      }
    }

    namespace UChar
    {
      Constant
      create(char32_t c)
      {
        return Constant(c, TYPE_INDEX_UCHAR);
      }

      Constant
      print(const Constant& c)
      {
        return String::create(
          u32string(1, get_constant<char32_t>(c)));
        ;
      }
    }

    namespace Intmp
    {
      Constant
      create(const mpz_class& i)
      {
        return make_constant_pointer
          (i, &intmp_type_functions, TYPE_INDEX_INTMP);
      }

      Constant
      create(int v)
      {
        return create(mpz_class(v));
      }

      Constant
      create(const Constant& text)
      {
        if (text.index() == TYPE_INDEX_USTRING)
        {
          try {
            return create(mpz_class(
              u32_to_ascii(get_constant_pointer<u32string>(text))));
          }
          catch (...)
          {
            return Types::Special::create(SP_CONST);
          }
        }
        else
        {
          return Types::Special::create(SP_CONST);
        }
      }

      const mpz_class&
      get(const Constant& i)
      {
        return get_constant_pointer<mpz_class>(i);
      }

      bool 
      equality(const Constant& lhs, const Constant& rhs)
      {
        return get(lhs) == get(rhs);
      }

      size_t
      hash(const Constant& c)
      {
        return std::hash<mpz_class>()(get(c));
      }

      void
      destroy(void* p)
      {
        delete reinterpret_cast<mpz_class*>(p);
      }

      Constant
      print(const Constant& c)
      {
        const mpz_class& z = get_constant_pointer<mpz_class>(c);

        if (z < 0)
        {
          mpz_class pos = -z;
          return Types::String::create(
            U"~" + to_u32string(pos.get_str()));
        }
        else
        {
          return Types::String::create(to_u32string(z.get_str()));
        }
      }
    }

    namespace Dimension
    {
      Constant
      create(dimension_index d)
      {
        return Constant(d, TYPE_INDEX_DIMENSION);
      }
    }

    namespace Hyperdatons
    {
      HD*
      get(const Constant& h)
      {
        return const_cast<HD*>(&get_constant_pointer<HD>(h));
      }

      Constant
      create(const HD* hd, type_index ti)
      {
        ConstantPointerValue* p = 
          new ConstantPointerValue(
            &hyperdaton_type_functions, 
            const_cast<HD*>(hd));
        return Constant(p, ti);
      }

      bool 
      equality(const Constant& lhs, const Constant& rhs)
      {
        return get(lhs) == get(rhs);
      }

      size_t
      hash(const Constant& c)
      {
        return reinterpret_cast<size_t>(get(c));
      }

      InputHD*
      getIn(const Constant& h)
      {
        return reinterpret_cast<InputHD*>(h.data.vptr);
      }
    }

    namespace Workshop
    {
      Constant
      create(const WS* ws)
      {
        ConstantPointerValue* p =
          new ConstantPointerValue(
            &workshop_type_functions,
            new WorkshopType(const_cast<WS*>(ws)));

        return Constant(p, TYPE_INDEX_WS);
      }

      const WorkshopType&
      get(const Constant& w)
      {
        return get_constant_pointer<WorkshopType>(w);
      }

      size_t
      hash(const Constant& c)
      {
        return reinterpret_cast<size_t>(get(c).ws());
      }

      bool
      equality(const Constant& lhs, const Constant& rhs)
      {
        return get(lhs).ws() == get(rhs).ws();
      }
    }

    namespace Demand
    {
      Constant
      create(std::vector<dimension_index>& dims)
      {
        return make_constant_pointer(DemandType(dims), &demand_type_functions, 
          TYPE_INDEX_DEMAND);
      }

      bool
      equality(const Constant& lhs, const Constant& rhs)
      {
        return get(lhs) == get(rhs);
      }

      size_t
      hash(const Constant& c)
      {
        return get(c).hash();
      }

      const DemandType&
      get(const Constant& c)
      {
        return get_constant_pointer<DemandType>(c);
      }
    }
  }
}

#if 0
const Special::StringValueInitialiser Special::m_sv;

Special::StringValueInitialiser::StringValueInitialiser()
: vtos
{
  {Special::ERROR, U"error"},
  {Special::ACCESS, U"access"},
  {Special::TYPEERROR, U"type"},
  {Special::DIMENSION, U"dim"},
  {Special::UNDEF, U"undef"},
  {Special::CONST, U"const"},
  {Special::LOOP, U"loop"},
  {Special::MULTIDEF, U"multidef"}
}
{
  u32string prefix({'s', 'p'});
  for(ValueStringMap::value_type const& v : vtos)
  {
    stov.insert(std::make_pair(v.second, v.first));
    u32string parser_string = prefix + v.second;

    parser_stov.insert(std::make_pair(parser_string, v.first));
  }
}
#endif

namespace TransLucid
{

Constant
BaseFunctionAbstraction::applyFn(const Constant& c) const
{
  Context newk;
  ContextPerturber p(newk, m_k);
  p.perturb(m_scope);
  p.perturb({{m_dim, c}});

  return (*m_expr)(newk);
}

Constant
ValueFunctionType::apply(Context& k, const Constant& value) const
{
  //set m_dim = value in the context and evaluate the expr
  ContextPerturber p(k, {{m_dim, value}});
  p.perturb(m_scopeDims);
  auto r = (*m_expr)(k);

  return r;
}

Constant
NameFunctionType::apply
(
  Context& k, 
  const Constant& c,
  std::vector<dimension_index>& Lall
) const
{
  //add to the list of our args
  //pre: c is a workshop value

  //add to the list of odometers
  tuple_t odometer;
  for (auto d : Lall)
  {
    odometer.insert(std::make_pair(d, k.lookup(d)));
  }

  //argdim = cons(c, #argdim)
  Tuple argList = makeList(c, k.lookup(m_argDim));
  Tuple odometerList = makeList(Types::Tuple::create(Tuple(odometer)),
    k.lookup(m_odometerDim));

  ContextPerturber p(k,
  {
    {m_argDim, Types::Tuple::create(argList)},
    {m_odometerDim, Types::Tuple::create(odometerList)}
  });

  p.perturb(m_scopeDims);

  return (*m_expr)(k);
}

//everything that creates hyperdatons
void
add_file_io(System& s)
{
  init_file_hds(s);
}

void
add_one_base_function(System& s, const u32string& name, BaseFunctionType* fn)
{
  std::unique_ptr<BangAbstractionWS> 
    op(new BangAbstractionWS(fn->clone()));

  //add equation fn.op_name = bang abstraction workshop with fn.fn
  s.addEquation(name, op.get());

  op.release();
}

inline
void
addTypeEquation(System& s, const u32string& type)
{
  s.addEquation(Parser::Equation(
    type,
    Tree::Expr(),
    Tree::Expr(),
    Tree::LiteralExpr(U"typetype", type)
  ));
}

inline
void
addTypeNames(System& s, const std::vector<u32string>& types)
{
  for (auto t : types)
  {
    addTypeEquation(s, t);
  }
  addTypeEquation(s, U"error");
}

void
add_builtin_literals(System& s, const std::vector<u32string>& types)
{
  for (auto t : types)
  {
    //constructor
    //removing with functions
    #if 0
    s.addEquation(Parser::Equation(
      U"LITERAL",
      Tree::TupleExpr({{Tree::DimensionExpr(type_name_dim), t}}),
      Tree::Expr(),
      Tree::BangAppExpr(Tree::IdentExpr(U"construct_" + t),
          Tree::HashExpr(Tree::DimensionExpr(U"text"))
      )
    ));

    //TYPENAME equation, go from value back to typename
    s.addEquation(Parser::Equation(
      U"TYPENAME",
      Tree::TupleExpr({{Tree::DimensionExpr(U"arg0"), Tree::IdentExpr(t)}}),
      Tree::Expr(),
      t)
    );
    #endif
  }

  BuiltinBaseFunction<1> construct_typetype{
    [&s] (const Constant& text) -> Constant
    {
      type_index t = s.getTypeIndex(get_constant_pointer<u32string>(text));

      return Types::Type::create(t);
    }
  };

  add_one_base_function(s, U"construct_typetype", &construct_typetype);

  s.addEquation(Parser::Equation(
    U"TYPENAME",
    Tree::TupleExpr({{Tree::DimensionExpr(U"arg0"), 
      Tree::IdentExpr(U"error")}}),
    Tree::Expr(),
    u32string(U"error"))
  );
}

void
add_builtin_printers(System& s, const std::vector<u32string>& to_print_types)
{
  //add all the printers for each type
  //PRINT | [arg0 : t] = "print_t"!(#arg0)

  for (auto t : to_print_types)
  {
    s.addEquation(Parser::Equation(
      PRINT_IDENT,
      Tree::TupleExpr({{Tree::DimensionExpr(U"arg0"), Tree::IdentExpr(t)}}),
      Tree::Expr(),
      Tree::BangAppExpr(Tree::IdentExpr(U"print_" + t),
          Tree::HashExpr(Tree::DimensionExpr(U"arg0"))
      )
    ));
  }

  //string returns itself
  //PRINT | [arg0 : ustring] = #arg0;;
  s.addEquation(Parser::Equation
  (
    PRINT_IDENT,
    Tree::TupleExpr(
    {{
      Tree::DimensionExpr(u32string(U"arg0")),
      Tree::IdentExpr(u32string(U"ustring"))
    }}),
    Tree::Expr(),
    Tree::HashExpr(Tree::DimensionExpr(u32string(U"arg0")))
  ));
}

void
add_base_functions(System& s)
{
  for (const auto& fn : fn_table)
  {
    add_one_base_function(s, fn.op_name, fn.fn);
  }
}


void
init_builtin_types(System& s)
{
  std::vector<u32string> to_print_types{
    U"bool",
    U"intmp",
    U"special",
    U"typetype",
    U"uchar",
    U"range",
    U"tuple"
  };

  std::vector<u32string> type_names = to_print_types;
  type_names.push_back(U"ustring");
  type_names.push_back(U"lambda");
  type_names.push_back(U"phi");
  type_names.push_back(U"uuid");
    
  //add all of the literals (LITERAL ... =)
  add_builtin_literals(s, type_names);

  //add all the definitions of t = type"t";;
  addTypeNames(s, type_names);

  to_print_types.push_back(U"error");

  add_builtin_printers(s, to_print_types);

  add_base_functions(s);

  add_file_io(s);
}

} //namespace TransLucid

namespace std
{
  #if 0
  template <>
  size_t
  hash<basic_string<unsigned int>>::operator()
  (const basic_string<unsigned int> s) const
  {
    size_t val = 0;
    for(unsigned int c : s)
    {
      val = _Hash_impl::__hash_combine(c, val);
    }
    return val;
  }
  #endif

  size_t
  hash<TransLucid::Special>::operator()
  (TransLucid::Special v) const
  {
    return v;
  }
}
