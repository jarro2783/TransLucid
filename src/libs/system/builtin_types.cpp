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
#include <tl/utility.hpp>

namespace TransLucid
{

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

void
String::print(std::ostream& os) const
{
  os << "ustring<" << utf32_to_utf8(m_s) << ">";
}

void
Special::print(std::ostream& os) const
{
  os << "special<" << utf32_to_utf8(m_sv.vtos.find(m_v)->second) << ">";
}

void Char::print(std::ostream& os) const
{
  u32string s(1, m_c);
  os << "uchar<" << utf32_to_utf8(s) << ">";
}

Guard::Guard(const GuardWS& g)
: m_g(new GuardWS(g))
{
}

Guard::Guard(const Guard& rhs)
: m_g(new GuardWS(*rhs.m_g))
{
}

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
  hash<TransLucid::Special::Value>::operator()
  (TransLucid::Special::Value v) const
  {
    return v;
  }
}
