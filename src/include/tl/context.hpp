/* The context.
   Copyright (C) 2009-2012 Jarryd Beck

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

/**
 * @file context.hpp
 * The context object.
 */

#ifndef TL_CONTEXT_HPP_INCLUDED
#define TL_CONTEXT_HPP_INCLUDED

#include <tl/types/special.hpp>
#include <tl/types.hpp>

#include <algorithm>
#include <deque>
#include <initializer_list>
#include <set>
#include <stack>
#include <vector>

namespace TransLucid
{
  class Context
  {
    public:

    Context();
    Context(const Tuple& k);

    operator Tuple() const;

    void
    perturb(const Tuple& t);

    void
    perturb(dimension_index d, const Constant& c);

    /**
     * Restores to the previous context.
     * Pops one value from every dimension that exists in t.
     * @param t The tuple to restore from.
     * @pre All of the dimensions in t have at least one value in the stack
     * of this; the behaviour is undefined otherwise.
     */
    void 
    restore(const Tuple& t);

    /**
     * Lookup a dimension.
     * If there is no value set, then use the all dimension.
     * @param dim The dimension index to lookup.
     * @return The ordinate of that dimension.
     */
    const Constant&
    lookup(dimension_index dim) const
    {
      if (dim <= m_min || dim >= m_max)
      {
        return m_all;
      }
      else
      {
        const auto& s = m_context[makeIndex(dim)];

        return s.first;
      }
    }

    template <typename List>
    void
    restore(const List& list)
    {
      for (const auto& v : list)
      {
        size_t index = makeIndex(v);
        m_context[index].second.pop();

        if (m_context[index].second.size() == 0)
        {
          m_setDims.erase(v);
          m_context[index].first = m_all;
        }
        else
        {
          m_context[index].first = m_context[index].second.top();
        }
      }
    }

    void
    reset();

    bool
    operator<=(const Context& rhs) const;

    bool
    has_entry(dimension_index i) const
    {
      return 
        i > m_min && 
        i < m_max && 
        m_context.at(makeIndex(i)).second.size() != 0;
    }

    dimension_index
    max() const
    {
      return m_max;
    }

    dimension_index
    min() const
    {
      return m_min;
    }

    const std::set<dimension_index>&
    setDims() const
    {
      return m_setDims;
    }

    void
    pushRho(uint8_t index)
    {
      m_rho.push_front(index);
    }

    void 
    popRho()
    {
      m_rho.pop_back();
    }

    uint8_t&
    topRho()
    {
      return m_rho.back();
    }

    const std::deque<uint8_t>&
    getRho()
    {
      return m_rho;
    }

    private:

    friend class ContextPerturber;

    dimension_index
    makeIndex(dimension_index i) const
    {
      return i - m_min - 1;
    }

    typedef std::deque<std::pair<Constant, std::stack<Constant>>> ContextType;

    //one before the smallest
    dimension_index m_min;
    //one after the biggest
    dimension_index m_max;

    Constant m_all;

    ContextType m_context;

    std::set<dimension_index> m_setDims;

    std::deque<uint8_t> m_rho;
  };

  class ContextPerturber
  {
    public:

    ContextPerturber
    (
      Context& k, 
      const std::initializer_list<std::pair<dimension_index, Constant>>& p
    )
    : m_k(k)
    {
      perturb(p);
    }

    ContextPerturber(Context& k)
    : m_k(k)
    {}

    template <typename T>
    ContextPerturber
    (
      Context& k,
      const T& t
    )
    : m_k(k)
    {
      perturb(t);
    }

    void perturb
    (
      dimension_index dim,
      const Constant& c
    )
    {
      m_k.perturb(dim, c);
      m_dims.push_back(dim);
    }

    void
    perturb
    (
      const std::initializer_list<std::pair<dimension_index, Constant>>& p
    )
    {
      for (const auto& v : p)
      {
        m_k.perturb(v.first, v.second);
        m_dims.push_back(v.first);
      }
    }

    template <typename T>
    void
    perturb(const T& t)
    {
      for (const auto& v : t)
      {
        m_k.perturb(v.first, v.second);
        m_dims.push_back(v.first);
      }
    }

    ContextPerturber
    (
      Context& k,
      const Tuple& delta
    )
    : m_k(k)
    {
      m_k.perturb(delta);
      for (const auto& v : delta)
      {
        m_dims.push_back(v.first);
      }
    }

    ~ContextPerturber()
    {
      m_k.restore(m_dims);
    }

    void
    perturb(const Context& k_p)
    {
      //perturb m_k by everything in k_p
      auto d = k_p.m_min + 1;
      auto iter = k_p.m_context.begin();

      while (iter != k_p.m_context.end())
      {
        if (!iter->second.empty())
        {
          perturb(d, iter->second.top());
        }

        ++iter;
        ++d;
      }
    }

    private:
    Context& m_k;
    std::vector<dimension_index> m_dims;
  };

  class MinimalContext
  {
    private:

    struct Comparator
    {
      bool
      operator()
      (
        dimension_index dim, 
        const std::pair<dimension_index, Constant>& p
      )
      const
      {
        return dim < p.first;
      }

      bool
      operator()
      (
        const std::pair<dimension_index, Constant>& p,
        dimension_index dim
      )
      const
      {
        return p.first < dim;
      }
    };

    public:

    MinimalContext
    (
      const Context& k
    )
    : m_all(Types::Special::create(SP_DIMENSION))
    {
      m_context.reserve(k.setDims().size());

      for (auto d : k.setDims())
      {
        m_context.push_back(std::make_pair(d, k.lookup(d)));
      }
    }

    const Constant&
    lookup(dimension_index dim) const
    {
      //binary search the context vector

      auto iter = std::lower_bound(m_context.begin(), m_context.end(), dim,
        Comparator()
      );

      if (iter != m_context.end() && iter->first == dim)
      {
        return iter->second;
      }
      else
      {
        return m_all;
      }
    }

    #if 0
    //computes this - rhs and puts the result in out
    template <typename OutputIterator>
    void
    difference(const Context& rhs, OutputIterator&& out) const
    {
      std::set_difference(
        m_setDims.begin(), m_setDims.end(),
        rhs.setDims().begin(), rhs.setDims().end(),
        std::forward<OutputIterator>(out)
      );
    }
    #endif

    //perturbs with p the things in this not in k
    void
    perturbDifference(ContextPerturber& p, Context& k) const
    {
      for (auto& pair : m_context)
      {
        if (!k.has_entry(pair.first))
        {
          p.perturb(pair.first, pair.second);
        }
      }
    }

    private:
    Constant m_all;
    std::vector<std::pair<dimension_index, Constant>> m_context;
  };

  class RhoManager
  {
    public:
    RhoManager(Context& k, uint8_t start = 0)
    : m_kappa(k)
    {
      m_kappa.pushRho(start);
    }

    ~RhoManager()
    {
      m_kappa.popRho();
    }

    void
    changeTop(int index)
    {
      m_kappa.topRho() = index;
    }

    private:
    Context& m_kappa;
  };

}

#endif
