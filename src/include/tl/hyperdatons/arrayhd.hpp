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

#include <tl/fixed_indexes.hpp>
#include <tl/hyperdaton.hpp>
#include <tl/types/special.hpp>
#include <tl/types_util.hpp>

#include <boost/multi_array.hpp>

namespace TransLucid
{
  struct array_get
  {
    #if 0
    template <typename Array, typename Index>
    auto
    operator()(const Array& a, Index i)
      -> decltype(a[i])
    {
      return a[i];
    }
    #endif

    template <typename Array>
    Array
    operator()(const Array& a)
    {
      return a;
    }

    template <typename Array, typename First, typename... Location>
    //typename std::result_of<array_get(Array, Location...)>::type
    auto
    operator()(const Array& a, First f, Location... loc)
      -> decltype(operator()(a[f], loc...))
    //  -> typename std::result_of<array_get(decltype(a[f]), Location...)>::type
    {
      return operator()(a[f], loc...);
    }
  };

  #if 0
  template <typename Array, typename Index>
  auto
  array_get(const Array& a, Index i)
    -> decltype(a[i])
  {
    return a[i];
  }

  template <typename Array, typename First, typename... Location>
  //typename std::result_of<array_get(Array, Location...)>::type
  auto
  array_get(const Array& a, First f, Location... loc)
    -> decltype(array_get(a[f], loc...))
  {
    return array_get(a[f], loc...);
  }
  #endif

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

    ~ArrayNHD() throw()
    {
    }

    Constant
    get(const Tuple& index) const
    {
      std::vector<int> vecIndex;
      for (auto d : m_dimensionOrder)
      {
        auto iter = index.find(d);
        if (iter == index.end())
        {
          //this should never happen
          throw "Invalid index in ArrayHD get: " __FILE__ ": " 
            STRING_(__LINE__);
        }
        else
        {
          //the preconditions of get are guaranteed by bestfitting
          vecIndex.push_back(
            get_constant_pointer<mpz_class>(iter->second).get_ui());
        }
      }

      return m_create(m_array(vecIndex));
    }

    template <typename... Location>
    auto
    //typename std::result_of<array_get(type, Location...)>::type
    get(Location... loc) const
      -> decltype(array_get()(m_array, loc...))
    {
      //return array_get(m_array, loc...);
      return array_get()(m_array, loc...);
    }

    void
    put (const Tuple& index, const Constant& v) override
    {
      //pull out the relevant variables of the tuple and access the array
      std::vector<int> vecIndex;
      for (auto d : m_dimensionOrder)
      {
        auto iter = index.find(d);
        if (iter == index.end())
        {
          return;
        }
        else
        {
          if (iter->second.index() == TYPE_INDEX_INTMP)
          {
            vecIndex.push_back(
              get_constant_pointer<mpz_class>(iter->second).get_ui());
          }
          else
          {
            return;
          }
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
  };
}

#endif
