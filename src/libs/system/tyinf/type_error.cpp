/* Type inference exceptions.
   Copyright (C) 2013 Jarryd Beck

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

#include <tl/tyinf/type_error.hpp>

namespace TransLucid
{

namespace TypeInference
{

u32string
SubcInvalid::print(System& system) const
{
  return print_type(c.lhs, system) + U" ≤ " + print_type(c.rhs, system);
}

u32string
InvalidConstraint::print(System& system) const
{
  return U"Invalid constraint: " + 
    print_type(c.lhs, system) + U" ≤ " + print_type(c.rhs, system);
}

u32string
BoundInvalid::print(System& system) const
{
  u32string result = print_type(a, system);

  if (type == GLB)
  {
    result += U" ⊓ ";
  }
  else
  {
    result += U" ⊔ ";
  }

  result += print_type(b, system);

  return result;
}

}

}
