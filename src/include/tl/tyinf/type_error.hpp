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

#include <tl/ast_fwd.hpp>

#include <tl/tyinf/type.hpp>
#include <tl/tyinf/constraint_graph.hpp>

namespace TransLucid
{
  namespace TypeInference
  {
    struct TypeError
    {
      virtual u32string
      print(System& system) const = 0;
    };

    struct SubcInvalid : public TypeError
    {
      SubcInvalid(Constraint con)
      : c(std::move(con))
      {
      }

      u32string
      print(System& system) const;

      Constraint c;
    };

    struct BoundInvalid : public TypeError
    {
      enum BoundType
      {
        GLB,
        LUB
      };

      BoundInvalid(BoundType t, Type a, Type b)
      : type(t), a(a), b(b)
      {
      }

      BoundType type;

      Type a;
      Type b;

      u32string
      print(System& system) const;
    };
    
    struct InvalidConstraint : public TypeError
    {
      InvalidConstraint(Constraint con)
      : c(std::move(con))
      {
      }

      u32string
      print(System& system) const;

      Constraint c;
    };

    struct RegionImpInvalid : public TypeError
    {
      RegionImpInvalid(const Tree::Expr& e)
      : e(e)
      {
      }

      u32string
      print(System& system) const;

      Tree::Expr e;
    };
  }
}
