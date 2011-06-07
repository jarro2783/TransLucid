/* Built-in types.
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

#include <tl/builtin_types.hpp>
#include <tl/equation.hpp>
#include <tl/types.hpp>
#include <tl/types/string.hpp>
#include <tl/utility.hpp>

namespace TransLucid
{

  namespace 
  {
    TypeFunctions string_type_functions =
      {
        &Types::String::equality,
        &Types::String::hash
      };
  }

  namespace Types
  {
    namespace String
    {
      Constant
      create(const u32string& s)
      {
        std::unique_ptr<u32string> value(new u32string(s));
        ConstantPointerValue* p = 
          new ConstantPointerValue(&string_type_functions, value.get());
        value.release();
        return Constant(p, TYPE_INDEX_USTRING);
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

#if 0

namespace TransLucid
{

//the default for function application is that there was a type mismatch
//concrete base classes will implement the correct functionality
TaggedConstant
FunctionType::applyLambda(const Tuple& k, const Constant& value) const
{
  return TaggedConstant(make_special(Special::CONST), k);
}

TaggedConstant
FunctionType::applyPhi(const Tuple& k, WS* expr) const
{
  return TaggedConstant(make_special(Special::CONST), k);
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

void
LambdaFunctionType::print(std::ostream& os) const
{
  os << "LambdaFunction";
}

}

#endif

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
