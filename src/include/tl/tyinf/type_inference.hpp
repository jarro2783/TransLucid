/* Type inference algorithm.
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

#ifndef TL_TYINF_TYPE_INFERENCE_HPP_INCLUDED
#define TL_TYINF_TYPE_INFERENCE_HPP_INCLUDED

#include <tl/ast.hpp>
#include <tl/tyinf/type.hpp>
#include <tl/tyinf/type_variable.hpp>

namespace TransLucid
{
  namespace TypeInference
  {
    class TypeInferrer
    {
      public:
      typedef Type result_type;

      result_type
      infer(const Tree::Expr& e);

      template <typename T>
      result_type
      operator()(const T& t)
      {
      }
    };
  }
}

#endif
