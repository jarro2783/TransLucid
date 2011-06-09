/* Physical hyperdatons.
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

#ifndef PHYSICAL_WSS_HPP_INCLUDED
#define PHYSICAL_WSS_HPP_INCLUDED

#include <tl/types.hpp>
#include <boost/multi_array.hpp>

namespace TransLucid
{
  class HD 
  {
    public:
    size_t
    lifetime() const
    {
      return m_lt;
    }

    protected:
    void
    init(size_t lt)
    {
      m_lt = lt;
    }

    size_t m_lt;
  };

  class InputHD : public virtual HD
  {
    public:
    InputHD(size_t lt)
    {
      this->init(lt);
    }

    virtual Constant
    get(const Tuple& k) const = 0;
  };

  class OutputHD : public virtual HD
  {
    public:
    OutputHD(size_t lt)
    {
      this->init(lt);
    }

    virtual void
    put(const Tuple& k, const Constant& c) = 0;
  };

  class IOHD : public InputHD, public OutputHD
  {
    public:
    IOHD(size_t lt)
    : InputHD(lt), OutputHD(lt)
    {
    }
  };

  template <typename Array, typename Index>
  auto
  array_get(const Array& a, Index i)
    -> decltype(a[i])
  {
    return a[i];
  }

  #if 0
  template <typename Array, typename First, typename... Location>
  auto 
  array_get(const Array& a, First f, Location... loc)
    -> decltype(array_get(a[f], loc...))
  {
    return array_get(a[f], loc...);
  }
  #endif

  template <typename Array, typename First, typename... Location>
  int
  array_get(const Array& a, First f, Location... loc)
    //-> decltype(array_get(a[f], loc...))
  {
    return array_get(a[f], loc...);
  }

  template <typename T, size_t N>
  class ArrayNHD
  {
    public:
    typedef boost::multi_array<T, N> type;

    type& get_array()
    {
      return m_array;
    }

    //if you get a compile error here, maybe all the types of Dimensions
    //weren't integers
    template <typename... Dimensions>
    ArrayNHD(Dimensions... dims)
    : m_array(std::vector<size_t>{dims...})
    {
      for (size_t i = 0; i != sizeof...(Dimensions); ++i)
      {
        m_dimensions.push_back(i);
      }
    }

    template <typename... Location>
    const T 
    get(Location... loc) const
    {
      return array_get(m_array, loc...);
    }

    private:
    type m_array;
    std::vector<mpz_class> m_dimensions;
  };

  template <size_t... Limits>
  class ArrayHD : public IOHD
  {
    private:
  };

  template <typename T, typename Wrapper>
  class Array0HD : public IOHD
  {
    public:
    Array0HD(size_t index)
    : IOHD(1)
    {
      m_data = new T;
    }

    ~Array0HD()
    {
      delete m_data;
    }

    const T&
    operator()() const
    {
      return *m_data;
    }

    void
    operator()(const T& v)
    {
      *m_data = v;
    }

    Constant
    get(const Tuple& k) const
    {
      return Constant(Wrapper(*m_data), index);
    }

    void
    put(const Tuple& k, const Constant& c)
    {
      try
      {
        //*m_data = c.value<T>().value();
      } catch (...)
      {
      }
    }

    private:
    T* m_data;
  };

  template <typename T, typename Wrapper>
  class Array1HD : public IOHD
  {
    public:
    Array1HD(size_t end0)
    : IOHD(1)
    , m_end0(end0)
    {
      m_data = new T[end0];
    }

    ~Array1HD()
    {
      delete [] m_data;
    }

    void
    operator()(size_t i0, const T& v)
    {
      m_data[i0] = v;
    }

    const T&
    operator()(size_t i0)
    {
      return m_data[i0];
    }

    Constant
    get(const Tuple& k) const
    {
      return Constant(Wrapper(*m_data), index);
    }

    void
    put(const Tuple& k, const Constant& c)
    {
      try
      {
        //*m_data = c.value<T>().value();
      } catch (...)
      {
      }
    }

    T* m_data;
    size_t m_end0;
  };

  template <typename T, typename Wrapper>
  class Array2HD : public IOHD
  {
    public:
    Array2HD(size_t end0, size_t end1)
    : IOHD(1)
    , m_end0(end0)
    , m_end1(end1)
    {
      m_data = new T[end0][end1];
    }

    ~Array2HD()
    {
      delete [] m_data;
    }

    void
    operator()(size_t i0, size_t i1, const T& v)
    {
      m_data[i0 * m_end1 + i1] = v;
    }

    const T&
    operator()(size_t i0, size_t i1)
    {
      return m_data[i0 * m_end1 + i1];
    }

    T* m_data;
    size_t m_end0, m_end1, m_end2;
  };

  template <typename T, typename Wrapper>
  class Array3HD : public IOHD
  {
    public:
    Array3HD(size_t end0, size_t end1, size_t end2)
    : IOHD(1)
    , m_end0(end0)
    , m_end1(end1)
    , m_end2(end2)
    {
      m_data = new T[end0][end1][end2];
    }

    ~Array3HD()
    {
      delete [] m_data;
    }

    void
    operator()(size_t i0, size_t i1, size_t i2, const T& v)
    {
      m_data[i0 * (m_end1*m_end2) + i1 * m_end2 + i2] = v;
    }

    const T&
    operator()(size_t i0, size_t i1, size_t i2)
    {
      return m_data[i0 * (m_end1*m_end2) + i1 * m_end2 + i2];
    }

    T* m_data;

    size_t m_end0, m_end1, m_end2;
  };

  template <typename T, typename Wrapper>
  class Array4HD : public IOHD
  {
    public:
    Array4HD(size_t end0, size_t end1, size_t end2, size_t end3)
    : IOHD(1)
    , m_end0(end0)
    , m_end1(end1)
    , m_end2(end2)
    , m_end3(end3)
    {
      m_data = new T[end0][end1][end2][end3];
    }

    ~Array4HD()
    {
      delete [] m_data;
    }

    void
    operator()(size_t i0, size_t i1, size_t i2, size_t i3, const T& v)
    {
      m_data[i0 * (m_end1*m_end2*m_end3) + 
             i1 * (m_end2*m_end3) + i2 * m_end3 + i3] = v;
    }

    const T&
    operator()(size_t i0, size_t i1, size_t i2, size_t i3)
    {
      return m_data[i0 * (m_end1*m_end2*m_end3) + 
                    i1 * (m_end2*m_end3) + i2 * m_end3 + i3];
    }

    T* m_data;
    size_t m_end0, m_end1, m_end2, m_end3;
  };
}

#endif
