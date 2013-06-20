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

#ifndef TL_TYINF_TYPE_HPP_INCLUDED
#define TL_TYINF_TYPE_HPP_INCLUDED

#include <vector>

#include <tl/types.hpp>
#include <tl/tyinf/type_variable.hpp>
#include <tl/variant.hpp>

namespace TransLucid
{
  namespace TypeInference
  {
    //The types are
    //tyvar
    //glb(t, ..., t)
    //lub(t, ..., t)
    //t ->^v t
    //(t_j) ->^b t
    //inten t
    //dim t
    //a .. b
    //c
    //top
    //bot
    //base type

    struct TypeGLB;
    struct TypeLUB;

    typedef Variant<
      TypeVariable,
      Constant,
      recursive_wrapper<TypeGLB>,
      recursive_wrapper<TypeLUB>
    > Type;

    struct TypeGLB
    {
      std::vector<Type> types;
    };

    struct TypeLUB
    {
      std::vector<Type> types;
    };

    Type
    construct_lub(Type a, Type b);

    Type
    construct_glb(Type a, Type b);

    // does a contain b
    bool
    type_term_contains(Type a, Type b);
  }
}

#endif
