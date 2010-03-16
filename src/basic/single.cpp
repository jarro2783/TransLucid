/* TODO: Give a descriptor.
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

#include <tl/equation.hpp>
#include <tl/interpreter.hpp>
#include <tl/consthd.hpp>
#include <tl/fixed_indexes.hpp>
#include <tl/utility.hpp>
#include <tl/builtin_types.hpp>
namespace TL = TransLucid;

using namespace TL;

int
main(int argc, char *argv[])
{
  TL::Interpreter i;
  HD& h = i;

  TL::Variable x(U"x", &i);
  TL::ConstHD::IntmpConst i1(5);
  TL::Tuple k;
  x.addExpr(k, &i1);

  //x = 5;;
  TL::TaggedValue r = x(k);
  std::cout << r.first.value<TL::Intmp>().value() << std::endl;

  //set up the equation guard
  TL::tuple_t guard;
  TL::ConstHD::IntmpConst i2(10);
  guard[DIM_VALUE] = TypedValue(TL::Intmp(10), TL::TYPE_INDEX_INTMP);

  //set up the context for addExpr
  tuple_t k2;
  k2[get_dimension_index(&h, U"_validguard")] =
     TypedValue(EquationGuardType(EquationGuard(Tuple(guard))),
                TYPE_INDEX_GUARD);

  //x @ [value : 10] = 10;;
  //add the new expression
  x.addExpr(Tuple(k2), &i2);

  //run the two to test best fitting
  r = x(k);
  std::cout << r.first.value<TL::Intmp>().value() << std::endl;

  r = x(Tuple(guard));
  std::cout << r.first.value<TL::Intmp>().value() << std::endl;
  return 0;
}
