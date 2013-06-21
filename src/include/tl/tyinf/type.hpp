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

    struct TagTop { };

    struct TagBot { };

    template <typename T>
    struct TypeNullary
    {
      bool
      operator==(const TypeNullary&) const
      {
        return true;
      }
    };

    typedef TypeNullary<TagTop> TypeTop;
    typedef TypeNullary<TagBot> TypeBot;

    typedef Variant<
      TypeTop,
      TypeBot,
      TypeVariable,
      Constant,
      recursive_wrapper<TypeGLB>,
      recursive_wrapper<TypeLUB>
    > Type;

    //a glb or lub type can never be equal
    struct TypeGLB
    {
      bool
      operator==(const TypeGLB& rhs) const
      {
        return false;
      }

      std::vector<Type> types;
    };

    struct TypeLUB
    {
      bool
      operator==(const TypeLUB& rhs) const
      {
        return false;
      }

      std::vector<Type> types;
    };

    Type
    construct_lub(Type a, Type b);

    Type
    construct_glb(Type a, Type b);

    // does t contain tp for negative types
    bool
    type_term_contains_neg(Type t, Type tp);

    // does t contain tp for positive types
    bool
    type_term_contains_pos(Type t, Type tp);
  }
}

#endif
