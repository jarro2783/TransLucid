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
#include <tl/types/function.hpp>
#include <tl/types/hyperdatons.hpp>
#include <tl/types/intmp.hpp>
#include <tl/types/range.hpp>
#include <tl/types/string.hpp>
#include <tl/types/tuple.hpp>
#include <tl/types/workshop.hpp>
#include <tl/types_util.hpp>
#include <tl/utility.hpp>

#include <gmpxx.h>

namespace TransLucid
{
  namespace
  {
    using namespace TransLucid::BuiltinOps;

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

    struct BuiltinFunction
    {
      const char32_t* abstract_name;
      const char32_t* op_name;
      BaseFunctionType* fn;
    };

    constexpr BuiltinFunction fn_table[] =
    {
      {U"plus", U"int_plus", &integer_plus},
      {U"minus", U"int_minus", &integer_minus},
      {U"times", U"int_times", &integer_times},
      {U"divide", U"int_divide", &integer_divide},
      {U"modulus", U"int_modulus", &integer_modulus},
      {U"lte", U"int_lte", &integer_lte},
      {U"lt", U"int_lt", &integer_lt},
      {U"gte", U"int_gte", &integer_gte},
      {U"gt", U"int_gt", &integer_gt},
      {U"eq", U"int_eq", &integer_eq},
      {U"ne", U"int_ne", &integer_ne},
      {U"plus", U"ustring_plus", &ustring_plus_fn},
    };
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
  }

  namespace
  {
    //all this does is return a bang abstraction type object with the
    //pointer that it is constructed with
    class BangAbstractionWS : public WS
    {
      public:
      constexpr BangAbstractionWS(BaseFunctionType* fn)
      : m_fn(fn)
      {
      }

      Constant
      operator()(Context& k)
      {
        return Types::BaseFunction::create(*m_fn);
      }

      private:
      BaseFunctionType* m_fn;
    };
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
}

void
add_builtin_literals(System& s, const std::vector<u32string>& types)
{
  //the type type
  #if 0
  s.addEquation(Parser::Equation(
    U"LITERAL",
    Tree::TupleExpr({{Tree::DimensionExpr(U"typename"), U"type"}}),
    Tree::Expr(),
    Tree::BangAppExpr(U"construct_type",
      {
        Tree::HashExpr(Tree::DimensionExpr(U"text")),
      }
    )
  ));
  #endif

  for (auto t : types)
  {
    //constructor
    s.addEquation(Parser::Equation(
      U"LITERAL",
      Tree::TupleExpr({{Tree::DimensionExpr(type_name_dim), t}}),
      Tree::Expr(),
      Tree::BangAppExpr(U"construct_" + t,
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
  }

  s.registerFunction(U"construct_typetype",
    make_function_type<1>::type(
      [&s] (const Constant& text) -> Constant
    {
      type_index t = s.getTypeIndex(get_constant_pointer<u32string>(text));

      if (t == 0)
      {
        return Types::Special::create(SP_CONST);
      }
      else
      {
        return Types::Type::create(t);
      }
    })
  );

  s.registerFunction(U"construct_intmp", 
    make_function_type<1>::type(
      static_cast<Constant (*)(const Constant&)>(Types::Intmp::create)));
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
      Tree::BangAppExpr(U"print_" + t,
          Tree::HashExpr(Tree::DimensionExpr(U"arg0"))
      )
    ));
  }

  s.registerFunction(U"print_intmp", 
    make_function_type<1>::type(&Types::Intmp::print));

  s.registerFunction(U"print_uchar",
    make_function_type<1>::type(&Types::UChar::print));

  s.registerFunction(U"print_special",
    make_function_type<1>::type(&Types::Special::print));

  s.registerFunction(U"print_bool",
    make_function_type<1>::type(&Types::Boolean::print));

  s.registerFunction(U"print_range",
    make_function_type<1>::type(&Types::Range::print));

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

template <typename F>
void
add_one_fun2
(
  System& s, 
  const u32string& fnname, 
  const u32string& sysop,
  const u32string& type,
  F func
)
{
  s.addEquation(Parser::Equation(
    FN2_IDENT,
    Tree::TupleExpr
    ({
      //{Tree::DimensionExpr(fnname_dim), u32string(U"plus")},
      {Tree::DimensionExpr(fnname_dim), fnname},
      {Tree::DimensionExpr(U"arg0"), Tree::IdentExpr(type)},
      {Tree::DimensionExpr(U"arg1"), Tree::IdentExpr(type)}
    }),
    Tree::Expr(),
    //u32string(U"int_plus")
    sysop
  ));

  s.registerFunction
  (
    //U"int_plus",
    //make_function_type<2>::type(&mpz_plus)
    sysop,
    make_function_type<2>::type(func)
  );
}

void
add_base_functions(System& s)
{
  for (const auto& fn : fn_table)
  {
    std::unique_ptr<BangAbstractionWS> 
      op(new BangAbstractionWS(fn.fn->clone()));

    //add equation fn.op_name = bang abstraction workshop with fn.fn
    s.addEquation(fn.op_name, op.get());

    op.release();
  }
}

void
add_builtin_ops(System& s)
{
  add_one_fun2(s, U"plus", U"int_plus", U"intmp", &mpz_plus);
  add_one_fun2(s, U"minus", U"int_minus", U"intmp", &mpz_minus);
  add_one_fun2(s, U"times", U"int_times", U"intmp", &mpz_times);
  add_one_fun2(s, U"divide", U"int_divide", U"intmp", &mpz_divide);
  add_one_fun2(s, U"modulus", U"int_modulus", U"intmp", &mpz_modulus);
  add_one_fun2(s, U"lte", U"int_lte", U"intmp", &mpz_lte);
  add_one_fun2(s, U"lt", U"int_lt", U"intmp", &mpz_lt);
  add_one_fun2(s, U"gte", U"int_gte", U"intmp", &mpz_gte);
  add_one_fun2(s, U"gt", U"int_gt", U"intmp", &mpz_gt);
  add_one_fun2(s, U"eq", U"int_eq", U"intmp", &mpz_eq);
  add_one_fun2(s, U"ne", U"int_ne", U"intmp", &mpz_ne);

  add_one_fun2(s, U"eq", U"bool_eq", U"bool", &bool_eq);
  add_one_fun2(s, U"ne", U"bool_ne", U"bool", &bool_ne);
  add_one_fun2(s, U"and", U"bool_and", U"bool", &bool_and);
  add_one_fun2(s, U"or", U"bool_or", U"bool", &bool_or);

  add_one_fun2(s, U"eq", U"ustring_eq", U"ustring", &ustring_eq);
  add_one_fun2(s, U"ne", U"ustring_ne", U"ustring", &ustring_ne);
  add_one_fun2(s, U"plus", U"ustring_plus", U"ustring", &ustring_plus);

  //range construction a..b
  add_one_fun2(s, U"range_construct", U"range_construct", U"intmp", 
    static_cast<Constant (*)(const Constant&, const Constant&)>
      (&Types::Range::create));

  add_base_functions(s);
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
    
  //add all of the literals (LITERAL ... =)
  add_builtin_literals(s, type_names);

  //add all the definitions of t = type"t";;
  addTypeNames(s, type_names);

  add_builtin_printers(s, to_print_types);

  add_builtin_ops(s);

  add_file_io(s);
}

} //namespace TransLucid

namespace std
{
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

  template <>
  size_t
  hash<TransLucid::Special>::operator()
  (TransLucid::Special v) const
  {
    return v;
  }
}
