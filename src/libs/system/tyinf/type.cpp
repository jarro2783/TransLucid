/* Types for type inference.
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

#include <tl/tyinf/type.hpp>

namespace TransLucid
{
namespace TypeInference
{

Type
construct_lub(Type a, Type b)
{
}

Type
construct_glb(Type a, Type b)
{
}

namespace
{
  struct ContainsCheck
  {
    typedef bool result_type;

    template <typename T>
    bool
    operator()(const T& t, Type b)
    {
      return false;
    }

    bool
    operator()(const TypeGLB& glb, Type b)
    {
    }

    bool 
    operator()(const TypeLUB& lub, Type b)
    {
    }
  };
}

bool
type_term_contains(Type a, Type b)
{
  //if (a == b)
  //{
  //  return true;
  //}

  ContainsCheck check;
  return apply_visitor(check, a, b);
}

}
}
