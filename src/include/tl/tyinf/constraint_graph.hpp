/* Constraint graph for type inference.
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

#ifndef TL_TYINF_CONSTRAINT_GRAPH_HPP_INCLUDED
#define TL_TYINF_CONSTRAINT_GRAPH_HPP_INCLUDED

#include <map>
#include <vector>

#include <tl/tyinf/type.hpp>
#include <tl/tyinf/type_variable.hpp>

namespace TransLucid
{
  namespace TypeInference
  {
    //a constraint graph is usually sparse, so it will be stored here as
    //a map from type variables alpha to a struct holding three things:
    //1. a list of <= type variables
    //2. the set of constraints in C\uparrow(alpha)
    //3. the set of constraints in C\downarrow(alpha)
    class ConstraintGraph
    {
      public:

      //Makes a union with another constraint graph.
      //The type variables must be disjoint.
      //The current one becomes the result.
      void
      make_union(const ConstraintGraph&);

      void
      add_constraint(TypeVariable a, TypeVariable b);

      void
      add_constraint(Type t, TypeVariable b);

      void
      add_constraint(TypeVariable a, Type t);

      //this one only works if t1 and t2 are one of the above three cases
      void
      add_constraint(Type t1, Type t2);

      private:

      struct ConstraintNode
      {
        std::vector<TypeVariable> less;
        Type pos;
        Type neg;
      };

      std::map<TypeVariable, ConstraintNode> m_graph;
    };
  }
}

#endif
