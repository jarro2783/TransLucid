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

#include <tl/output.hpp>
#include <tl/workshop.hpp>

namespace TransLucid
{

  class IntensionType
  {
    public:
    IntensionType
    (
      System* system,
      WS* ws, 
      const std::vector<Constant> binds,
      const std::vector<dimension_index>& scope,
      Context& k
    )
    : m_system(system)
    , m_ws(ws)
    , m_binds(binds)
    , m_scope(scope)
    , m_k(k.minimal_copy())
    {
    }

    WS* ws() const
    {
      return m_ws;
    }

    Constant
    operator()(Context& k_a) const
    {
      // m_k \dagger k_a \dagger (m_k <| {\rho, m_scope})
      //
      // which is implemented by
      //
      // k_a \dagger (k_a - m_k) \dagger (m_k <| {\rho, m_scope})

      //make this more efficient

      std::vector<std::pair<dimension_index, Constant>> saved;
      saved.reserve(m_scope.size() + 1);

      for (auto b : m_binds)
      {
        auto dim = m_system->getDimensionIndex(b);
        saved.push_back(std::make_pair(dim, m_k.lookup(dim)));
      }

      for (auto d : m_scope)
      {
        saved.push_back(std::make_pair(d, m_k.lookup(d)));
      }

      saved.push_back(std::make_pair(DIM_RHO, m_k.lookup(DIM_RHO)));

      ContextPerturber p(m_k, k_a); 

      p.perturb(saved);

      auto result = (*m_ws)(m_k);

      return result;
    }

    private:
    System* m_system;
    WS* m_ws;
    std::vector<Constant> m_binds;
    std::vector<dimension_index> m_scope;
    mutable Context m_k;
  };

  namespace Types
  {
    namespace Intension
    {
      Constant
      create
      (
        System* system,
        WS* ws, 
        std::vector<Constant> binds,
        const std::vector<dimension_index>& scope,
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
