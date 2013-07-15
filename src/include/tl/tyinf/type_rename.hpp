/* Type scheme rewriter.
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

#ifndef TL_TYPE_RENAME_HPP_INCLUDED
#define TL_TYPE_RENAME_HPP_INCLUDED

#include <tl/tyinf/type_traverse.hpp>
#include <tl/tyinf/type_inference.hpp>

namespace TransLucid
{
  namespace TypeInference
  {
    class Rename : public GenericTypeWalker
    {
      public:

      typedef Type result_type;

      using GenericTypeWalker::operator();

      Rename(FreshTypeVars& fresh)
      : m_fresh(fresh)
      {
      }

      TypeScheme
      rename(const TypeScheme& t)
      {
        using std::placeholders::_1;

        TypeContext A = TypeContext::rewrite(std::get<0>(t),
          std::bind(visitor_applier(), std::ref(*this), _1));

        ConstraintGraph C = std::get<2>(t);
        C.rewrite_bounds
        (
          std::bind(visitor_applier(), std::ref(*this), _1),
          std::bind(visitor_applier(), std::ref(*this), _1)
        );

        auto tr = apply_visitor(*this, std::get<1>(t));

        return std::make_tuple(A, tr, C);
      }

      result_type
      operator()(TypeVariable v)
      {
        return rename_typevar(v);
      }

//finish this
#if 0
      result_type
      operator()(const TypeGLB& glb)
      {
      }

      result_type
      operator()(const TypeLUB& lub)
      {
      }
#endif

      TypeVariable
      rename_typevar(TypeVariable v)
      {
        auto iter = m_renames.find(v);
        
        if (iter == m_renames.end())
        {
          auto t = m_fresh.fresh();
          m_renames.insert(std::make_pair(v, t));

          return t;
        }
        else
        {
          return iter->second;
        }
      }

      private:
      FreshTypeVars& m_fresh;

      std::map<TypeVariable, TypeVariable> m_renames;
    };
  }
}

#endif
