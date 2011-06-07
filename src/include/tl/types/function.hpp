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

    virtual TaggedConstant
    applyLambda(const Tuple& k, const Constant& value) const;

    virtual TaggedConstant
    applyPhi(const Tuple& k, WS* expr) const;
  };

  class LambdaFunctionType : public FunctionType
  {
    public:
    LambdaFunctionType(const u32string& name, dimension_index dim, WS* expr)
    : m_name(name), m_dim(dim), m_expr(expr)
    {
    }

    TaggedConstant
    applyLambda(const Tuple& k, const Constant& value) const;

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

      FunctionType&
      get(const Constant& c);
    }
  }
}

#endif


