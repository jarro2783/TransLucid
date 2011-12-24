/* Array hyperdaton.
   Copyright (C) 2011 Jarryd Beck.

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

#ifndef TL_ARRAYHD_HPP_INCLUDED
#define TL_ARRAYHD_HPP_INCLUDED

#include <gmpxx.h>

#include <tl/fixed_indexes.hpp>
#include <tl/hyperdaton.hpp>
#include <tl/types/special.hpp>
#include <tl/types_util.hpp>

#include <boost/multi_array.hpp>

namespace TransLucid
{
  struct array_get
  {
    template <typename Array>
    Array
    operator()(const Array& a)
    {
      return a;
    }

    template <typename Array, typename First, typename... Location>
    auto
    operator()(const Array& a, First f, Location... loc)
      -> decltype(operator()(a[f], loc...))
    {
      return operator()(a[f], loc...);
    }
  };

  template <typename T, size_t N>
  class ArrayNHD : public IOHD
  {
    public:
    typedef boost::multi_array<T, N> type;

    void
    commit() {}

    type& get_array()
    {
      return m_array;
    }
    private:
    type m_array;
    std::vector<mpz_class> m_bounds;
    Tuple m_variance;
    std::vector<dimension_index> m_dimensionOrder;
    std::function<Constant(const T&)> m_create;
    std::function<T(const Constant&)> m_get_func;

    public:
    //if you get a compile error here, maybe all the types of Dimensions
    //weren't integers

    /**
     * Construct an ArrayNHD. Takes the name of the type in the array
     * and the array dimensions.
     * @param bounds The size of the array.
     * @param dims The dimensions of variance.
     * @param typeReg The type registry.
     * @param dimReg The dimension registry.
     * @param typeName The name of the type being stored.
     * @note N = bounds = dim
     */
    template <typename Create, typename Get>
    ArrayNHD
    (
      const std::vector<size_t>& bounds, 
      const std::vector<Constant> dims,
      DimensionRegistry& dimReg,
      Create create,
      Get getF
    )
    : IOHD(1), m_array(bounds), m_create(create), m_get_func(getF)
    {
      assert(N == bounds.size() && N == dims.size());

      tuple_t variance;
      for (size_t i = 0; i != bounds.size(); ++i)
      {
        dimension_index d = dimReg.getDimensionIndex(dims[i]);
        m_bounds.push_back(bounds[i]);
        m_dimensionOrder.push_back(d);

        //create variance tuple
        mpz_class a = 0;
        mpz_class b = m_bounds[i]-1;
        variance.insert(std::make_pair(d,
          Types::Range::create(Range(&a, &b))
        ));
      }

      m_variance = variance;
    }

    //this is currently causing an ICE in GCC
    //swap the lines around to see, I can't reproduce it at the moment
    ~ArrayNHD() = default;
    //~ArrayNHD() throw() { }

    Constant
    get(const Context& index) const
    {
      std::vector<int> vecIndex;
      for (auto d : m_dimensionOrder)
      {
        const Constant& val = index.lookup(d);
        if (val.index() != TYPE_INDEX_INTMP)
        {
          //this should never happen
          throw "Invalid index in ArrayHD get: " __FILE__ ": " 
            STRING_(__LINE__);
        }
        else
        {
          //the preconditions of get are guaranteed by bestfitting
          vecIndex.push_back(
            get_constant_pointer<mpz_class>(val).get_ui());
        }
      }

      return m_create(m_array(vecIndex));
    }

    template <typename... Location>
    auto
    get(Location... loc) const
      -> decltype(array_get()(m_array, loc...))
    {
      return array_get()(m_array, loc...);
    }

    void
    put (const Context& index, const Constant& v) override
    {
      //pull out the relevant variables of the tuple and access the array
      std::vector<int> vecIndex;
      for (auto d : m_dimensionOrder)
      {
        const Constant& cindex = index.lookup(d);
        if (cindex.index() == TYPE_INDEX_INTMP)
        {
          vecIndex.push_back(
            get_constant_pointer<mpz_class>(cindex).get_ui());
        }
        else
        {
          return;
        }
      }

      m_array(vecIndex) = m_get_func(v);
    }

    auto
    operator[](int i)
      -> decltype(m_array[i])
    {
      return m_array[i];
    }

    Tuple
    variance() const
    {
      return m_variance;
    }

    void
    addAssignment(const Tuple&)
    {
      //ignore because this is a fixed sized array
    }
  };
}

#endif
