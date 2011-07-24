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
#include <tl/internal_strings.hpp>
#include <tl/system.hpp>
#include <tl/types.hpp>
#include <tl/types/function.hpp>
#include <tl/types/intmp.hpp>
#include <tl/types/range.hpp>
#include <tl/types/string.hpp>
#include <tl/types/tuple.hpp>
#include <tl/types/uuid.hpp>
#include <tl/types_util.hpp>
#include <tl/utility.hpp>

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

    class TypeConstructor : public WS
    {
      public:

      TypeConstructor(System& s)
      : m_system(s)
      {
      }

      TaggedConstant
      operator()(const Tuple& k);

      private:
      System& m_system;
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
        return get(lhs) == get(rhs);
      }
    }

    namespace Boolean
    {
      Constant
      create(bool v)
      {
        return Constant(v, TYPE_INDEX_BOOL);
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
    }

    namespace Range
    {
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
    }

    namespace Intmp
    {
      Constant
      create(const mpz_class& i)
      {
        return make_constant_pointer
          (i, &intmp_type_functions, TYPE_INDEX_INTMP);
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
        return boost::hash<mpz_class>()(get(c));
      }

      void
      destroy(void* p)
      {
        delete reinterpret_cast<mpz_class*>(p);
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
    s.addEquation(Parser::Equation(
      U"LITERAL",
      Tree::TupleExpr({{Tree::DimensionExpr(U"typename"), t}}),
      Tree::Expr(),
      Tree::BangOpExpr(U"construct_" + t,
        {
          Tree::HashExpr(Tree::DimensionExpr(U"text")),
        }
      )
    ));
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
}

void
init_builtin_types(System& s)
{
  //this must be sorted
  std::vector<u32string> type_names{
    U"bool",
    U"intmp",
    U"special"
    U"type",
    U"uchar",
    U"ustring",
  };
    
  //add all of the literals (LITERAL ... =)
  add_builtin_literals(s, type_names);

  //add all the definitions of t = type"t";;
  addTypeNames(s, type_names);

  //add all the printers for each type
  //PRINT | [arg0 : t] = "print_t"!(#arg0)

  //remove "ustring" from the types
  std::vector<u32string> to_print_types;
  std::vector<u32string> string{U"ustring"};
  std::set_difference(type_names.begin(), type_names.end(),
    string.begin(), string.end(),
    std::back_inserter(to_print_types));

  #if 0
  for (auto t : type_names)
  {
    s.addEquation(Parser::Equation(
      U"PRINT",
      Tree::TupleExpr({{Tree::DimensionExpr(U"arg0"), Tree::IdentExpr(t)}}),
      Tree::Expr(),
      Tree::BangOpExpr(U"print_" + t,
        {
          Tree::HashExpr(Tree::DimensionExpr(U"arg0")),
        }
      )
    ));
  }
  #endif

  s.registerFunction(U"print_intmp",
    make_function_type<1>::type(
    [] (const Constant& i) -> Constant
    {
      return Types::String::create(
        to_u32string(get_constant_pointer<mpz_class>(i).get_str()));
    }
    )
  );

  //string returns itself
  //PRINT | [arg0 : ustring] = #arg0;;
  s.addEquation(Parser::Equation
  (
    U"PRINT",
    Tree::TupleExpr(
    {{
      Tree::DimensionExpr(u32string(U"arg0")),
      Tree::IdentExpr(u32string(U"ustring"))
    }}),
    Tree::Expr(),
    Tree::HashExpr(Tree::DimensionExpr(u32string(U"arg0")))
  ));
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
