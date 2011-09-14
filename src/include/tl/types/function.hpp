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

  class LambdaFunctionType : public FunctionType
  {
    public:
    LambdaFunctionType(const u32string& name, dimension_index dim, WS* expr)
    : m_name(name), m_dim(dim), m_expr(expr)
    {
    }

    Constant
    applyLambda(Context& k, const Constant& value) const;

    LambdaFunctionType*
    clone() const
    {
      return new LambdaFunctionType(*this);
    }

    size_t
    hash() const
    {
      return reinterpret_cast<size_t>(m_expr);
    }

    private:
    u32string m_name;
    dimension_index m_dim;
    WS* m_expr;
  };

  class PhiFunctionType : public FunctionType
  {
    private:
    WS* m_expr;
  };


  namespace Types
  {
    namespace Function
    {
      Constant
      create(const FunctionType& f);

      const FunctionType&
      get(const Constant& c);

      bool
      equality(const Constant& lhs, const Constant& rhs);

      size_t
      hash(const Constant& c);
    }
  }
}

#endif


