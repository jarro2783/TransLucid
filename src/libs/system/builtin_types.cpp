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

#include <tl/builtin_types.hpp>
#include <tl/equation.hpp>
#include <tl/hyperdatons/filehd.hpp>
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
#include <tl/types/uuid.hpp>
#include <tl/types_util.hpp>
#include <tl/utility.hpp>

#include <gmpxx.h>

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
  namespace 
  {
    TypeFunctions string_type_functions =
      {
        &Types::String::equality,
        &Types::String::hash,
        &delete_ptr<u32string>
      };

    TypeFunctions function_type_functions =
      {
        &Types::Function::equality,
        &Types::Function::hash,
        &delete_ptr<FunctionType>
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

    TypeFunctions uuid_type_functions =
      {
        &Types::UUID::equality,
        &Types::UUID::hash,
        &delete_ptr<uuid>
      };

    TypeFunctions hyperdaton_type_functions =
      {
        &Types::Hyperdatons::equality,
        &Types::Hyperdatons::hash,
        &delete_ptr<HD>
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

    //the following functions will create the appropriate file hyperdaton
    //which contains an appropriate fstream object and then wrap it up
    //as a constant
    inline Constant
    open_input_file(type_index ti, const u32string& file)
    {
      FileInputHD* in = new FileInputHD(file);

      return Types::Hyperdatons::create(in, ti);
    }

    inline Constant
    open_output_file(type_index ti, const u32string& file)
    {
      //TODO implement me
      return Types::Special::create(SP_ERROR);
    }

    inline Constant
    open_io_file(type_index ti, const u32string& file)
    {
      //TODO implement me
      return Types::Special::create(SP_ERROR);
    }

    struct FileOpener
    {
      FileOpener(type_index in, type_index out, type_index io)
      : m_in(in), m_out(out), m_io(io)
      {
      }

      Constant
      operator()(const Constant& file, const Constant& mode)
      {
        if (mode.index() != TYPE_INDEX_INTMP || 
            file.index() != TYPE_INDEX_USTRING)
        {
          return Types::Special::create(SP_TYPEERROR);
        }

        const mpz_class& intmode = get_constant_pointer<mpz_class>(mode);
        const u32string& sfile = get_constant_pointer<u32string>(file);

        switch (intmode.get_ui())
        {
          case 1:
          //input
          return open_input_file(m_in, sfile);
          break;

          case 2:
          //output
          return open_output_file(m_out, sfile);
          break;

          case 3:
          //io
          return open_io_file(m_out, sfile);
          break;

          default:
          return Types::Special::create(SP_DIMENSION);
        }
      }

      type_index m_in, m_out, m_io;
    };

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
    struct clone<FunctionType>
    {
      FunctionType*
      operator()(const FunctionType& v)
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
    struct clone<uuid>
    {
      uuid*
      operator()(const uuid& u)
      {
        return new uuid(u);
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

    namespace Function
    {
      Constant
      create(const FunctionType& f)
      {
        return make_constant_pointer
          (f, &function_type_functions, TYPE_INDEX_FUNCTION);
      }

      const FunctionType&
      get(const Constant& c)
      {
        return get_constant_pointer<FunctionType>(c);
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

    namespace UUID
    {
      Constant
      create(const uuid& i)
      {
        return make_constant_pointer
          (i, &uuid_type_functions, TYPE_INDEX_UUID);
      }

      const uuid&
      get(const Constant& u)
      {
        return get_constant_pointer<uuid>(u);
      }

      bool 
      equality(const Constant& lhs, const Constant& rhs)
      {
        return get(lhs) == get(rhs);
      }

      size_t
      hash(const Constant& c)
      {
        return boost::hash<uuid>()(get(c));
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
        return Types::String::create(
          to_u32string(get_constant_pointer<mpz_class>(c).get_str()));
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
      const HD*
      get(const Constant& h)
      {
        return &get_constant_pointer<HD>(h);
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

//the default for function application is that there was a type mismatch
//concrete base classes will implement the correct functionality
TaggedConstant
FunctionType::applyLambda(const Tuple& k, const Constant& value) const
{
  return TaggedConstant(Types::Special::create(SP_CONST), k);
}

TaggedConstant
FunctionType::applyPhi(const Tuple& k, WS* expr) const
{
  return TaggedConstant(Types::Special::create(SP_CONST), k);
}

FunctionType::~FunctionType()
{
}

TaggedConstant
LambdaFunctionType::applyLambda(const Tuple& k, const Constant& value) const
{
  //set m_dim = value in the context and evaluate the expr
  tuple_t k_f = k.tuple();
  k_f[m_dim] = value;
  return (*m_expr)(Tuple(k_f));
}

//everything that creates hyperdatons
void
add_file_io(System& s)
{
  type_index in, out, io;

  in = s.getTypeIndex(U"inhd");
  out = s.getTypeIndex(U"outhd");
  io = s.getTypeIndex(U"iohd");

  s.registerFunction(U"fileopen", 
    make_function_type<2>::type(
      FileOpener(in, out, io)
    )
  );

  //file [arg0 : string, arg1 : intmp] = "openfile"!(#arg0, #arg1);;
  //ifile [arg0 : string] = file @ [arg1 <- 1];;
  //ofile [arg0 : string] = file @ [arg1 <- 2];;
  //iofile [arg0 : string] = file @ [arg1 <- 3];;
}

inline
void
addTypeEquation(System& s, const u32string& type)
{
  s.addEquation(Parser::Equation(
    type,
    Tree::Expr(),
    Tree::Expr(),
    Tree::LiteralExpr(U"type", type)
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
    Tree::BangOpExpr(U"construct_type",
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
      Tree::BangOpExpr(U"construct_" + t,
        {
          Tree::HashExpr(Tree::DimensionExpr(U"text")),
        }
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

  s.registerFunction(U"construct_type",
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
      Tree::BangOpExpr(U"print_" + t,
        {
          Tree::HashExpr(Tree::DimensionExpr(U"arg0")),
        }
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
}

void
init_builtin_types(System& s)
{
  std::vector<u32string> to_print_types{
    U"bool",
    U"intmp",
    U"special",
    U"type",
    U"uchar",
    U"range"
  };

  std::vector<u32string> type_names = to_print_types;
  type_names.push_back(U"ustring");
    
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
