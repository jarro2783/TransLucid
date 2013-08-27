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
    template <typename Self>
    class GenericTypeTransformer
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
      operator()(const TypeDim& dim)
      {
        return TypeDim{apply_visitor(*reinterpret_cast<Self*>(this),
          dim.body)};
      }

      Type
      operator()(const TypeIntension& i)
      {
        return TypeIntension{apply_visitor(*reinterpret_cast<Self*>(this), 
          i.body)};
      }

      Type
      operator()(const TypeCBV& cbv)
      {
        return TypeCBV
        {
          apply_visitor(*reinterpret_cast<Self*>(this), cbv.lhs), 
          apply_visitor(*reinterpret_cast<Self*>(this), cbv.rhs)
        };
      }

      Type
      operator()(const TypeBase& base)
      {
        std::vector<Type> args;

        for (const auto& t : base.lhs)
        {
          args.push_back(apply_visitor(*reinterpret_cast<Self*>(this), t));
        }

        return TypeBase
        {
          args,
          apply_visitor(*reinterpret_cast<Self*>(this), base.rhs)
        };
      }
    };

    template <typename Self>
    class GenericTypeTraverser
    {
      public:

      typedef void result_type;

      template <typename T>
      void
      operator()(const T& t) const
      {
      }

      void
      operator()(const TypeDim& dim)
      {
        apply_visitor(*reinterpret_cast<Self*>(this), dim.body);
      }

      void
      operator()(const TypeIntension& i)
      {
        apply_visitor(*reinterpret_cast<Self*>(this), i.body);
      }

      void
      operator()(const TypeCBV& cbv)
      {
        apply_visitor(*reinterpret_cast<Self*>(this), cbv.lhs);
        apply_visitor(*reinterpret_cast<Self*>(this), cbv.rhs);
      }

      void
      operator()(const TypeBase& base)
      {
        for (const auto& t : base.lhs)
        {
          apply_visitor(*reinterpret_cast<Self*>(this), t);
        }

        apply_visitor(*reinterpret_cast<Self*>(this), base.rhs);
      }
    };
  }
}

#endif
