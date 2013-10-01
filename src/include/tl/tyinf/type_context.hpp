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
      addDimension(TypeVariable dim, 
        std::pair<Type, Type> value)
      {
        auto iter = m_dimensions.find(dim);

        if (iter == m_dimensions.end())
        {
          iter = m_dimensions.insert(std::make_pair(dim, 
            std::make_pair(Type(), Type()))).first;
        }

        iter->second = std::make_pair(
          construct_lub(iter->second.first, value.first),
          construct_glb(iter->second.second, value.second)
        );
      }

      void
      add(dimension_index d, Type t);

      void
      remove(dimension_index d)
      {
        m_lambdas.erase(d);
      }

      void
      remove(const u32string& x)
      {
        m_vars.erase(x);
      }

      void
      add(const u32string& x, Type t);

      //inserts other into the current
      void
      join(const TypeContext& other);

      Type
      lookup(dimension_index d);

      Type
      lookup(const u32string& x);

      bool
      has_entry(dimension_index d);

      bool
      has_entry(const u32string& x);

      template <typename Rewriter>
      static
      TypeContext
      rewrite(const TypeContext& c, Rewriter r)
      {
        TypeContext result;

        for (const auto& v : c.m_lambdas)
        {
          result.m_lambdas.insert(std::make_pair(v.first, 
            r.rewrite_type(v.second)));
        }

        for (const auto& v : c.m_vars)
        {
          result.m_vars.insert(std::make_pair(v.first, 
            r.rewrite_type(v.second)));
        }

        for (auto v : c.m_dimensions)
        {
          auto rewritten = r.rename_var(v.first);
          result.addDimension(rewritten, 
            std::make_pair(
              r.rewrite_type(v.second.first), 
              r.rewrite_type(v.second.second)
            )
          );
        }

        return result;
      }

      template <typename F>
      void
      for_each(F f) const
      {
        for (const auto& v : m_lambdas)
        {
          f(v.second);
        }

        for (const auto& v : m_vars)
        {
          f(v.second);
        }
      }

      private:
      std::map<dimension_index, Type> m_lambdas;
      std::unordered_map<u32string, Type> m_vars;
      std::map<TypeVariable, std::pair<Type, Type>> m_dimensions;
      //std::vector<std::pair<TypeVariable, TypeVariable>> m_dimensions;

      public:
      const decltype(m_dimensions)&
      getDimensions() const
      {
        return m_dimensions;
      }
    };
  }
}

#endif
