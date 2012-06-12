/* Intension type, stores a pointer to workshop with scope in a Constant.
   Copyright (C) 2011, 2012 Jarryd Beck.

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

#ifndef TL_TYPES_WORKSHOP_HPP_INCLUDED
#define TL_TYPES_WORKSHOP_HPP_INCLUDED

#include <tl/workshop.hpp>

namespace TransLucid
{

  class IntensionType
  {
    public:
    IntensionType
    (
      WS* ws, 
      const std::vector<std::pair<dimension_index, Constant>>& scope,
      Context& k
    )
    : m_ws(ws), m_k(k)
    {
    }

    WS* ws() const
    {
      return m_ws;
    }

    Constant
    operator()(Context& k) const
    {
      // k \dagger k_a \dagger (k <| {\rho, m_scope})
      //
      // which is implemented by
      //
      // k_a \dagger (k_a - k) \dagger (k <| {\rho, m_scope})

      #warning implement me
      ContextPerturber p(k, m_scope);
      return (*m_ws)(k);
    }

    private:
    WS* m_ws;
    std::vector<std::pair<dimension_index, Constant>> m_scope;
    Context m_k;
  };

  namespace Types
  {
    namespace Intension
    {
      Constant
      create
      (
        WS* ws, 
        const std::vector<std::pair<dimension_index, Constant>>& scope,
        Context& k
      );

      const IntensionType&
      get(const Constant& c);

      bool 
      equality(const Constant& lhs, const Constant& rhs);

      size_t
      hash(const Constant& c);

      bool
      less(const Constant& lhs, const Constant& rhs);
    }
  }
}

#endif
