/* Built-in types.
   Copyright (C) 2009--2012 Jarryd Beck

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
#include <tl/system_util.hpp>
#include <tl/types.hpp>
#include <tl/types/boolean.hpp>
#include <tl/types/char.hpp>
#include <tl/types/demand.hpp>
#include <tl/types/floatmp.hpp>
#include <tl/types/function.hpp>
#include <tl/types/hyperdatons.hpp>
#include <tl/types/intmp.hpp>
#include <tl/types/numbers.hpp>
#include <tl/types/range.hpp>
#include <tl/types/region.hpp>
#include <tl/types/string.hpp>
#include <tl/types/tuple.hpp>
#include <tl/types/type.hpp>
#include <tl/types/union.hpp>
#include <tl/types/uuid.hpp>
#include <tl/types/intension.hpp>
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

    BuiltinBaseFunction<1> construct_floatmp {
      static_cast<Constant (*)(const Constant&)>(&Types::Floatmp::create)
    };

    BuiltinBaseFunction<1> print_intmp{&Types::Intmp::print};
    BuiltinBaseFunction<1> print_uchar{&Types::UChar::print};
    //BuiltinBaseFunction<1> print_special_base{&Types::Special::print};
    BuiltinBaseFunction<1> print_bool{&Types::Boolean::print};
    BuiltinBaseFunction<1> print_range{&Types::Range::print};
    BuiltinBaseFunction<1> print_typetype{&Types::Type::print};
    BuiltinBaseFunction<1> print_error{&print_error_value};
    BuiltinBaseFunction<1> print_uuid{&Types::UUID::print};
    BuiltinBaseFunction<1> print_floatmp{&Types::Floatmp::print};

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
    BuiltinBaseFunction<1> integer_uminus{&mpz_uminus};

    BuiltinBaseFunction<2> float_plus{&mpf_plus};
    BuiltinBaseFunction<2> float_minus{&mpf_minus};
    BuiltinBaseFunction<2> float_times{&mpf_times};
    BuiltinBaseFunction<2> float_divide{&mpf_divide};
    BuiltinBaseFunction<2> float_lte{&mpf_lte};
    BuiltinBaseFunction<2> float_lt{&mpf_lt};
    BuiltinBaseFunction<2> float_gte{&mpf_gte};
    BuiltinBaseFunction<2> float_gt{&mpf_gt};
    BuiltinBaseFunction<2> float_eq{&mpf_eq};
    BuiltinBaseFunction<2> float_ne{&mpf_ne};
    BuiltinBaseFunction<1> float_sqrt{&mpf_sqrt};
    BuiltinBaseFunction<1> float_abs{&mpf_abs};
    BuiltinBaseFunction<1> float_uminus{&mpf_uminus};
    BuiltinBaseFunction<1> float_convert_intmp{&mpf_convert_intmp};

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

    BuiltinBaseFunction<2> construct_union{&Types::Union::create};

    struct BuiltinFunction
    {
      const char32_t* op_name;
      BaseFunctionType* fn;
    };

    constexpr BuiltinFunction fn_table[] =
    {
      {U"intmp_plus", &integer_plus},
      {U"intmp_minus", &integer_minus},
      {U"intmp_times", &integer_times},
      {U"intmp_divide", &integer_divide},
      {U"intmp_modulus", &integer_modulus},
      {U"intmp_lte", &integer_lte},
      {U"intmp_lt", &integer_lt},
      {U"intmp_gte", &integer_gte},
      {U"intmp_gt", &integer_gt},
      {U"intmp_eq", &integer_eq},
      {U"intmp_ne", &integer_ne},
      {U"intmp_uminus", &integer_uminus},
      
      {U"floatmp_plus", &float_plus},
      {U"floatmp_minus", &float_minus},
      {U"floatmp_times", &float_times},
      {U"floatmp_divide", &float_divide},
      {U"floatmp_lte", &float_lte},
      {U"floatmp_lt", &float_lt},
      {U"floatmp_gte", &float_gte},
      {U"floatmp_gt", &float_gt},
      {U"floatmp_eq", &float_eq},
      {U"floatmp_ne", &float_ne},
      {U"floatmp_sqrt", &float_sqrt},
      {U"floatmp_abs", &float_abs},
      {U"floatmp_uminus", &float_uminus},
      {U"floatmp_convert_intmp", &float_convert_intmp},

      {U"bool_eq", &boolean_eq},
      {U"ustring_plus", &ustring_plus_fn},
      {U"make_range", &range_create},
      {U"make_range_infty", &range_create_inf},
      {U"make_range_neginfty", &range_create_neginf},
      {U"make_range_infinite", &range_create_infinity},
      {U"construct_intmp", &construct_integer},
      {U"construct_special", &construct_special},
      {U"construct_uuid", &construct_uuid},
      {U"construct_floatmp", &construct_floatmp},
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
      {U"print_floatmp", &print_floatmp},
      {U"make_union", &construct_union},
    };

    bool
    less_false(const Constant&, const Constant&)
    {
      return false;
    }
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

  template <>
  struct hash<mpf_class>
  {
    size_t
    operator()(const mpf_class& f) const
    {
      mp_exp_t exp;
      std::string s = f.get_str(exp);
      return std::hash<std::string>()(s);
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
        &delete_ptr<u32string>,
        &Types::String::less
      };

    TypeFunctions base_function_type_functions =
      {
        &Types::BaseFunction::equality,
        &Types::BaseFunction::hash,
        &delete_ptr<BaseFunctionType>,
        &less_false
      };

    TypeFunctions value_function_type_functions =
      {
        &Types::ValueFunction::equality,
        &Types::ValueFunction::hash,
        &delete_ptr<ValueFunctionType>,
        &Types::ValueFunction::less
      };

    TypeFunctions range_type_functions =
      {
        &Types::Range::equality,
        &Types::Range::hash,
        &delete_ptr<Range>,
        &Types::Range::less
      };

    TypeFunctions region_type_functions =
      {
        &Types::Region::equality,
        &Types::Region::hash,
        &delete_ptr<Region>,
        &Types::Region::less
      };

    TypeFunctions tuple_type_functions =
      {
        &Types::Tuple::equality,
        &Types::Tuple::hash,
        &delete_ptr<Tuple>,
        &Types::Tuple::less
      };

    TypeFunctions intmp_type_functions = 
      {
        &Types::Intmp::equality,
        &Types::Intmp::hash,
        &delete_ptr<mpz_class>,
        &Types::Intmp::less
      };

    TypeFunctions floatmp_type_functions = 
      {
        &Types::Floatmp::equality,
        &Types::Floatmp::hash,
        &delete_ptr<mpf_class>,
        &Types::Floatmp::less
      };

    TypeFunctions hyperdaton_type_functions =
      {
        &Types::Hyperdatons::equality,
        &Types::Hyperdatons::hash,
        &delete_ptr<HD>
      };

    TypeFunctions workshop_type_functions =
      {
        &Types::Intension::equality,
        &Types::Intension::hash,
        &delete_ptr<IntensionType>,
        &Types::Intension::less
      };

    TypeFunctions demand_type_functions =
      {
        &Types::Demand::equality,
        &Types::Demand::hash,
        &delete_ptr<DemandType>,
        &Types::Demand::less
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
      {U"spdim", SP_DIMENSION},
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
    mpz_uminus(const Constant& a)
    {
      return Types::Intmp::create(-Types::Intmp::get(a));
    }

    Constant
    mpf_plus(const Constant& a, const Constant& b)
    {
      return Types::Floatmp::create(get_constant_pointer<mpf_class>(a) +
        get_constant_pointer<mpf_class>(b))
      ;
    }

    Constant
    mpf_minus(const Constant& a, const Constant& b)
    {
      return Types::Floatmp::create(get_constant_pointer<mpf_class>(a) -
        get_constant_pointer<mpf_class>(b))
      ;
    }

    Constant
    mpf_times(const Constant& a, const Constant& b)
    {
      return Types::Floatmp::create(get_constant_pointer<mpf_class>(a) *
        get_constant_pointer<mpf_class>(b))
      ;
    }

    Constant
    mpf_divide(const Constant& a, const Constant& b)
    {
      mpf_class bval = get_constant_pointer<mpf_class>(b);
      if (bval == 0)
      {
        return Types::Special::create(Special::SP_ARITH);
      }
      else
      {
        return Types::Floatmp::create(get_constant_pointer<mpf_class>(a) /
          bval);
      }
      ;
    }

    Constant
    mpf_lte(const Constant& a, const Constant& b)
    {
      return Types::Boolean::create(get_constant_pointer<mpf_class>(a) <=
        get_constant_pointer<mpf_class>(b))
      ;
    }

    Constant
    mpf_lt(const Constant& a, const Constant& b)
    {
      return Types::Boolean::create(get_constant_pointer<mpf_class>(a) <
        get_constant_pointer<mpf_class>(b))
      ;
    }

    Constant
    mpf_gte(const Constant& a, const Constant& b)
    {
      return Types::Boolean::create(get_constant_pointer<mpf_class>(a) >=
        get_constant_pointer<mpf_class>(b))
      ;
    }

    Constant
    mpf_gt(const Constant& a, const Constant& b)
    {
      return Types::Boolean::create(get_constant_pointer<mpf_class>(a) >
        get_constant_pointer<mpf_class>(b))
      ;
    }

    Constant
    mpf_eq(const Constant& a, const Constant& b)
    {
      return Types::Boolean::create(get_constant_pointer<mpf_class>(a) ==
        get_constant_pointer<mpf_class>(b))
      ;
    }

    Constant
    mpf_ne(const Constant& a, const Constant& b)
    {
      return Types::Boolean::create(get_constant_pointer<mpf_class>(a) !=
        get_constant_pointer<mpf_class>(b))
      ;
    }

    Constant
    mpf_sqrt(const Constant& f)
    {
      return Types::Floatmp::create(sqrt(Types::Floatmp::get(f)));
    }

    Constant
    mpf_abs(const Constant& f)
    {
      return Types::Floatmp::create(abs(Types::Floatmp::get(f)));
    }

    Constant
    mpf_uminus(const Constant& f)
    {
      return Types::Floatmp::create(-Types::Floatmp::get(f));
    }

    Constant
    mpf_convert_intmp(const Constant& i)
    {
      return Types::Floatmp::create(Types::Intmp::get(i));
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
    struct clone<mpf_class>
    {
      mpf_class*
      operator()(const mpf_class& i)
      {
        return new mpf_class(i);
      }
    };

    template <>
    struct clone<Region>
    {
      Region*
      operator()(const Region& r)
      {
        return new Region(r);
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

      bool
      less(const Constant& lhs, const Constant& rhs)
      {
        return get(lhs) < get(rhs);
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

      bool
      less(const Constant& lhs, const Constant& rhs)
      {
        return get(lhs) < get(rhs);
      }
    }

    namespace Region
    {
      Constant
      create(const TransLucid::Region& r)
      {
        return make_constant_pointer
          (r, &region_type_functions, TYPE_INDEX_REGION);
      }

      const TransLucid::Region&
      get(const Constant& r)
      {
        return get_constant_pointer<TransLucid::Region>(r);
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

      bool
      less(const Constant& lhs, const Constant& rhs)
      {
        return get(lhs) < get(rhs);
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

      bool
      less(const Constant& lhs, const Constant& rhs)
      {
        return get(lhs) < get(rhs);
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

    namespace Floatmp
    {
      const mpf_class&
      get(const Constant& c)
      {
        return get_constant_pointer<mpf_class>(c);
      }

      Constant
      create(const mpf_class& f)
      {
        return make_constant_pointer
          (f, &floatmp_type_functions, TYPE_INDEX_FLOATMP);
      }

      Constant
      create(const Constant& text)
      {
        if (text.index() == TYPE_INDEX_USTRING)
        {
          try {
            return create(mpf_class(
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
      
      bool 
      equality(const Constant& lhs, const Constant& rhs)
      {
        return get(lhs) == get(rhs);
      }

      size_t
      hash(const Constant& c)
      {
        return std::hash<mpf_class>()(get(c));
      }

      Constant
      print(const Constant& c)
      {
        const mpf_class& f = get(c);

        std::string s;
        mp_exp_t exp;

        s = f.get_str(exp);

        std::string result;

        std::ostringstream os;

        if (s[0] == '-')
        {
          os << "-0." << s.substr(1, std::string::npos);
        }
        else
        {
          os << "0." << s;
        }
        os << "e" << exp;
        result = os.str();

        #if 0
        if (exp == 0)
        {
          result = s;
        }
        else if (exp > 0)
        {
          result.append(s.begin(), s.begin() + exp);
          result.push_back('.');
          result.append(s.begin() + exp + 1, s.end());
        }
        else
        {
          result.append("0.");
          result.append(exp, '0');
          result.append(s);
        }
        #endif

        return Types::String::create(u32string(result.begin(), result.end()));
      }

      bool
      less(const Constant& lhs, const Constant& rhs)
      {
        return get(lhs) < get(rhs);
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

      bool
      less(const Constant& lhs, const Constant& rhs)
      {
        return get(lhs) < get(rhs);
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

    namespace Intension
    {
      Constant
      create
      (
        System* system,
        WS* ws, 
        std::vector<Constant> binds,
        std::vector<dimension_index> scope,
        Context& k
      )
      {
        ConstantPointerValue* p =
          new ConstantPointerValue(
            &workshop_type_functions,
            new IntensionType(system, const_cast<WS*>(ws), 
              std::move(binds), 
              std::move(scope), k));

        return Constant(p, TYPE_INDEX_INTENSION);
      }

      const IntensionType&
      get(const Constant& w)
      {
        return get_constant_pointer<IntensionType>(w);
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

      bool
      less(const Constant& lhs, const Constant& rhs)
      {
        return get(lhs).ws() < get(rhs).ws();
      }
    }

    namespace Demand
    {
      Constant
      create(const std::vector<dimension_index>& dims)
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

      bool
      less(const Constant& lhs, const Constant& rhs)
      {
        const auto& lhsd = get_constant_pointer<DemandType>(lhs);
        const auto& rhsd = get_constant_pointer<DemandType>(rhs);

        return lhsd.dims() < rhsd.dims();
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
  if (m_dims.size() != 1)
  {
    return Types::Special::create(SP_TYPEERROR);
  }

  Context newk;
  ContextPerturber p(newk, m_binds);
  p.perturb({{m_dims.at(0), c}});

  return (*m_expr)(newk);
}

//everything that creates hyperdatons
void
add_file_io(System& s)
{
  //init_file_hds(s);
}

void
add_one_base_function(System& s, const u32string& name, BaseFunctionType* fn)
{
  s.addHostFunction(name, fn, fn->arity());
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
}

void
add_builtin_printers(System& s, const std::vector<u32string>& to_print_types)
{
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
    U"tuple",
    U"demand",
    U"calc",
    U"floatmp"
  };

  std::vector<u32string> type_names = to_print_types;
  type_names.push_back(U"ustring");
  type_names.push_back(U"basefun");
  type_names.push_back(U"lambda");
  type_names.push_back(U"phi");
  type_names.push_back(U"uuid");
  type_names.push_back(U"intension");
    
  //add all of the literals (LITERAL ... =)
  add_builtin_literals(s, type_names);

  //add all the definitions of t = type"t";;
  addTypeNames(s, type_names);

  to_print_types.push_back(U"error");

  //add_builtin_printers(s, to_print_types);

  add_base_functions(s);

  add_file_io(s);

  registerIntegers(s);
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
