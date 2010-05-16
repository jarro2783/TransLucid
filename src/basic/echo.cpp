/* Simple echo input test.
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

#include <tl/interpreter.hpp>
#include <boost/assign/list_of.hpp>
#include <iostream>
#include <tl/builtin_types.hpp>
#include <tl/fixed_indexes.hpp>
#include <tl/compiled_functors.hpp>
#include <tl/utility.hpp>

using namespace TransLucid;
using boost::assign::map_list_of;

class Receiver : public HD
{
  public:
  TaggedConstant operator()(const Tuple& k);

  uuid addExpr(const Tuple& k, HD *h);

  private:
  TaggedConstant m_value;
};

class Sender : public HD
{
  public:
  TaggedConstant operator()(const Tuple& k);

  uuid addExpr(const Tuple& k, HD* h);

  private:
  static const int BUF_SIZE = 1000;
  char32_t m_buf[BUF_SIZE];
};

TaggedConstant
Receiver::operator()(const Tuple& k)
{
  return m_value;
}

uuid
Receiver::addExpr(const Tuple& k, HD* h)
{
  Tuple::const_iterator iter = k.find(DIM_VALUE);
  m_value = TaggedConstant(iter->second, k);
  return boost::uuids::random_generator()();
}

TaggedConstant
Sender::operator()(const Tuple& k)
{
  //std::cin.getline(m_buf, BUF_SIZE);
  //return TaggedConstant(Constant(String(m_buf), TYPE_INDEX_USTRING), k);
}

uuid
Sender::addExpr(const Tuple& k, HD* h)
{
  return boost::uuids::random_generator()();
}

int
main(int argc, char* argv[])
{
  Interpreter i;

  Receiver r;
  Sender s;

  i.addOutput(map_list_of(U"out", &r));
  i.addInput(map_list_of(U"keyboard", &s));

  i.addDemand(U"out", EquationGuard());

  //set out = keyboard
  Hyperdatons::IdentHD ident(&i, U"keyboard");
  tuple_t context =
    map_list_of(size_t(DIM_ID), generate_string(U"out"))
               (get_dimension_index(&i, U"_validguard"),
                Constant(EquationGuardType(EquationGuard()),
                TYPE_INDEX_GUARD));
  i.addExpr(Tuple(context), &ident);

  while (true)
  {
    i.tick();
    TaggedConstant result = r(Tuple());
    //std::cout << result.first.value<String>().value() << std::endl;
  }

  return 0;
}
