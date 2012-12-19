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
      const std::vector<Constant>& binds,
      const std::vector<dimension_index>& scope,
      Context& k
    )
    : m_system(system)
    , m_ws(ws)
    , m_binds(std::move(binds))
    , m_scope(std::move(scope))
    {
      for (auto c : binds)
      {
        auto d = m_system->getDimensionIndex(c);
        m_bound.push_back(std::make_pair(d, k.lookup(d)));
      }

      for (auto d : scope)
      {
        m_bound.push_back(std::make_pair(d, k.lookup(d)));
      }

      m_bound.push_back(std::make_pair(DIM_RHO, k.lookup(DIM_RHO)));
    }

    WS* ws() const
    {
      return m_ws;
    }

    Constant
    operator()(Context& k_a) const
    {
      // new semantics
      // k_a \dagger (k <| {m_binds, m_scope, \rho})
      // so only the dimensions requested from kappa will be saved

      ContextPerturber p(k_a); 

      p.perturb(m_bound);

      auto result = (*m_ws)(k_a);

      return result;
    }

    TimeConstant
    operator()(Context& kappa, Delta& d, const Thread& w, size_t t) const
    {
      ContextPerturber pk(kappa); 
      DeltaPerturber pd(d);

      pk.perturb(m_bound);
      pd.perturb(m_bound);

      return (*m_ws)(kappa, d, w, t);
    }

    private:
    System* m_system;
    WS* m_ws;
    std::vector<Constant> m_binds;
    std::vector<dimension_index> m_scope;
    std::vector<std::pair<dimension_index, Constant>> m_bound;
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
        std::vector<dimension_index> scope,
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
