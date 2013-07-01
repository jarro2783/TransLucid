/* Type context.
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

#ifndef TL_TYINF_TYPE_CONTEXT_HPP_INCLUDED
#define TL_TYINF_TYPE_CONTEXT_HPP_INCLUDED

#include <map>
#include <unordered_map>

#include <tl/types.hpp>
#include <tl/tyinf/type.hpp>

namespace TransLucid
{
  namespace TypeInference
  {
    class TypeContext
    {
      public:

      void
      add(dimension_index d, Type t);

      void
      remove(dimension_index d)
      {
        m_lambdas.erase(d);
      }

      //inserts other into the current
      void
      join(const TypeContext& other);

      Type
      lookup(dimension_index d);

      template <typename Rewriter>
      static
      TypeContext
      rewrite(const TypeContext& c, Rewriter r)
      {
        TypeContext result;

        for (const auto& v : c.m_lambdas)
        {
          result.m_lambdas.insert(std::make_pair(v.first, r(v.second)));
        }

        for (const auto& v : c.m_vars)
        {
          result.m_vars.insert(std::make_pair(v.first, r(v.second)));
        }

        return result;
      }

      private:
      std::map<dimension_index, Type> m_lambdas;
      std::unordered_map<u32string, Type> m_vars;
    };
  }
}

#endif
