/* Type tree traversal.
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

#ifndef TL_TYPE_TRAVERSE_HPP_INCLUDED
#define TL_TYPE_TRAVERSE_HPP_INCLUDED

#include <tl/tyinf/type.hpp>

namespace TransLucid
{
  namespace TypeInference
  {
    class GenericTypeWalker
    {
      public:

      typedef Type result_type;

      template <typename T>
      Type
      operator()(const T& t) const
      {
        return t;
      }

      Type
      operator()(const TypeIntension& i)
      {
        return TypeIntension{apply_visitor(*this, i.body)};
      }

      Type
      operator()(const TypeCBV& cbv)
      {
        return TypeCBV
        {
          apply_visitor(*this, cbv.lhs), 
          apply_visitor(*this, cbv.rhs)
        };
      }

      Type
      operator()(const TypeBase& base)
      {
        std::vector<Type> args;

        for (const auto& t : base.lhs)
        {
          args.push_back(apply_visitor(*this, t));
        }

        return TypeBase
        {
          args,
          apply_visitor(*this, base.rhs)
        };
      }
    };
  }
}

#endif
