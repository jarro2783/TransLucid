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

    //bound types
    struct TypeGLB;
    struct TypeLUB;

    //function types
    struct TypeCBV;
    struct TypeIntension;
    struct TypeBase;

    //tags for single value empty types
    struct TagTop { };
    struct TagBot { };
    struct TagNothing { };

    //single value empty types
    template <typename T>
    struct TypeNullary
    {
      bool
      operator==(const TypeNullary&) const
      {
        return true;
      }
    };

    //the type for a whole implementation atomic type
    struct TypeAtomic
    {
      type_index index;

      bool
      operator==(const TypeAtomic& rhs) const
      {
        return index == rhs.index;
      }
    };

    typedef TypeNullary<TagTop> TypeTop;
    typedef TypeNullary<TagBot> TypeBot;
    typedef TypeNullary<TagNothing> TypeNothing;

    typedef Variant<
      TypeNothing,
      TypeTop,
      TypeBot,
      TypeVariable,
      Constant,
      TypeAtomic,
      recursive_wrapper<TypeGLB>,
      recursive_wrapper<TypeLUB>,
      recursive_wrapper<TypeIntension>,
      recursive_wrapper<TypeCBV>,
      recursive_wrapper<TypeBase>
    > Type;

    //a glb or lub type can never be equal
    struct TypeGLB
    {
      bool
      operator==(const TypeGLB& rhs) const
      {
        return vars == rhs.vars && constructed == rhs.constructed;
      }

      std::set<TypeVariable> vars;
      Type constructed;
    };

    struct TypeLUB
    {
      bool
      operator==(const TypeLUB& rhs) const
      {
        return vars == rhs.vars && constructed == rhs.constructed;
      }

      std::set<TypeVariable> vars;
      Type constructed;
    };

    struct TypeCBV
    {
      Type lhs;
      Type rhs;

      bool
      operator==(const TypeCBV& other) const
      {
        return lhs == other.lhs && rhs == other.rhs;
      }
    };

    struct TypeBase
    {
      std::vector<Type> lhs;
      Type rhs;

      bool operator==(const TypeBase& other) const
      {
        return lhs == other.lhs && rhs == other.rhs;
      }
    };

    struct TypeIntension
    {
      Type body;

      bool
      operator==(const TypeIntension& rhs) const
      {
        return body == rhs.body;
      }
    };

    //constructs and normalises
    Type
    construct_lub(Type a, Type b);

    //constructs and normalises
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
