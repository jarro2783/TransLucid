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
  class System;

  namespace TypeInference
  {
    class ConstraintGraph;

    class TypeContext
    {
      private:

      #if 0
      template <typename T, typename Key>
      void
      addDimension
      (
        T& container, 
        const Key& k, 
        const Type& lower, 
        const Type& upper
      )
      {
        auto iter = container.find(k);

        if (iter == container.end())
        {
          container.insert(std::make_pair(k, std::make_pair(lower, upper)));
        }
        else
        {
          iter->second = std::make_pair(
            construct_lub(iter->second.first, lower),
            construct_glb(iter->second.second, upper)
          );
        }
      }
      #endif

      public:

      void
      addConstantDim(const Constant& c, const Type& lower, const Type& upper)
      {
        auto iter = m_constDims.find(c);

        if (iter == m_constDims.end())
        {
          m_constDims.insert(std::make_pair(c, std::make_pair(lower, upper)));
        }
        else
        {
          iter->second = std::make_pair(
            construct_lub(iter->second.first, lower),
            construct_glb(iter->second.second, upper)
          );
        }
      }

      void
      addParamDim(dimension_index d, const Type& value,
        const Type& lower, const Type& upper)
      {
        auto iter = m_paramDims.find(d);

        if (iter == m_paramDims.end())
        {
          m_paramDims.insert(std::make_pair(d, 
            std::make_tuple(value, lower, upper)));
        }
        else
        {
          iter->second = std::make_tuple(
            construct_lub(std::get<0>(iter->second), value),
            construct_lub(std::get<1>(iter->second), lower),
            construct_glb(std::get<2>(iter->second), upper)
          );
        }
      }

      template <typename Pos, typename Neg>
      void
      markTLContext(Pos&& pos, Neg&& neg) const
      {
        for (const auto& c : m_constDims)
        {
          pos(c.second.first);
          neg(c.second.second);
        }

        for (const auto& c : m_paramDims)
        {
          pos(std::get<0>(c.second));
          pos(std::get<1>(c.second));
          neg(std::get<2>(c.second));
        }
      }

      void
      instantiateDim(dimension_index param, const Constant& value);

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
      remove(const Constant& c)
      {
        m_constDims.erase(c);
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

      std::pair<Type, Type>
      lookup(const Constant& c);

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

        for (const auto& v : c.m_constDims)
        {
          result.addConstantDim(v.first, 
            r.rewrite_type(v.second.first), 
            r.rewrite_type(v.second.second)
          );
        }

        for (const auto& v : c.m_paramDims)
        {
          result.addParamDim(v.first, 
            r.rewrite_type(std::get<0>(v.second)), 
            r.rewrite_type(std::get<1>(v.second)), 
            r.rewrite_type(std::get<2>(v.second))
          );
        }

        return result;
      }

      template <typename Rewriter>
      static
      TypeContext
      canonise(const TypeContext& c, const Rewriter& pos, const Rewriter& neg)
      {
        TypeContext result;

        for (auto& v : c.m_lambdas)
        {
          result.m_lambdas.insert(std::make_pair(v.first, 
            neg.rewrite_type(v.second))); 
        }

        for (auto& v : c.m_vars)
        {
          result.m_vars.insert(std::make_pair(v.first, 
            neg.rewrite_type(v.second)));
        }

        for (auto& v : c.m_paramDims)
        {
          result.addParamDim(v.first,
            pos.rewrite_type(std::get<0>(v.second)),
            pos.rewrite_type(std::get<1>(v.second)),
            neg.rewrite_type(std::get<2>(v.second))
          );
        }

        for (auto& v : c.m_constDims)
        {
          result.addConstantDim(v.first,
            pos.rewrite_type(v.second.first),
            neg.rewrite_type(v.second.second)
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

      u32string
      print_context(System& system);

      void
      fix_context(ConstraintGraph& C);

      void
      instantiate_parameters(ConstraintGraph& C);

      void
      clear_context()
      {
        m_constDims.clear();
        m_paramDims.clear();
      }

      void
      clear_raw_context()
      {
        m_constDims.clear();
      }

      private:
      std::map<dimension_index, Type> m_lambdas;
      std::unordered_map<u32string, Type> m_vars;
      std::map<Constant, std::pair<Type, Type>> m_constDims;
      std::map<dimension_index, std::tuple<Type, Type, Type>> m_paramDims;
    };
  }
}

#endif
