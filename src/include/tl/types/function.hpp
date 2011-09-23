/* The function types.
   Copyright (C) 2011 Jarryd Beck and John Plaice

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

#ifndef TYPES_FUNCTION_HPP_INCLUDED
#define TYPES_FUNCTION_HPP_INCLUDED

#include <tl/types.hpp>

namespace TransLucid
{
  class FunctionType
  {
    public:
    virtual ~FunctionType() = 0;

    virtual Constant
    applyLambda(Context& k, const Constant& value) const;

    virtual Constant
    applyPhi(Context& k, WS* expr) const;

    virtual FunctionType*
    clone() const = 0;

    virtual size_t
    hash() const = 0;
  };

  class ValueFunctionType
  {
    public:
    ValueFunctionType
    (
      const u32string& name, 
      dimension_index argDim, 
      const std::vector<dimension_index>& valueScope,
      const std::vector<dimension_index>& namedScopeArgs,
      const std::vector<dimension_index>& namedScopeOdometers,
      WS* expr,
      const Context& k
    )
    : m_name(name), m_dim(argDim), m_expr(expr)
    {
      for (auto d : valueScope)
      {
        m_scopeDims.push_back(std::make_pair(d, k.lookup(d)));
      }

      for (auto d : namedScopeArgs)
      {
        m_scopeDims.push_back(std::make_pair(d, k.lookup(d)));
      }

      for (auto d : namedScopeOdometers)
      {
        m_scopeDims.push_back(std::make_pair(d, k.lookup(d)));
      }
    }

    ValueFunctionType*
    clone() const
    {
      return new ValueFunctionType(*this);
    }

    Constant
    apply(Context& k, const Constant& value) const;

    size_t
    hash() const
    {
      return reinterpret_cast<size_t>(m_expr);
    }

    private:
    u32string m_name;
    dimension_index m_dim;
    std::vector<std::pair<dimension_index, Constant>> m_scopeDims;
    WS* m_expr;
  };

  class PhiFunctionType : public FunctionType
  {
    private:
    WS* m_expr;
  };


  namespace Types
  {
    namespace ValueFunction
    {
      Constant
      create(const ValueFunctionType& f);

      const ValueFunctionType&
      get(const Constant& c);

      bool
      equality(const Constant& lhs, const Constant& rhs);

      size_t
      hash(const Constant& c);
    }
  }
}

#endif


