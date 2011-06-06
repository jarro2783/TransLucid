/* All the types in the system.
   Copyright (C) 2009, 2010 Jarryd Beck and John Plaice

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

#ifndef TYPES_H_INCLUDED
#define TYPES_H_INCLUDED

#include <tl/types_fwd.hpp>

#include <boost/functional/hash.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>

#include <iostream>
#include <map>
#include <vector>

#include <gmpxx.h>

/**
 * @file types.hpp
 * All of the core type definitions in the TransLucid interpreter. These
 * are the types which are crucial for the operation of the interpreter,
 * specifically Constant and Tuple.
 */

#define STRING(x) #x
#define STRING_(x) STRING(x)

inline size_t
hash_value(const mpz_class& v)
{
  boost::hash<std::string> hasher;
  return hasher(v.get_str());
}

namespace boost
{
  namespace uuids
  {
    inline std::ostream&
    operator<<(std::ostream& os, const uuid& id)
    {
      for(int i : id)
      {
        os << std::hex << i;
      }
      return os;
    }
  }
}

/**
 * @brief The namespace that all of the TransLucid library is placed in.
 **/
namespace TransLucid
{

  class System;

  class WS;

  //class GuardWS;

  typedef uint16_t type_index;
  typedef std::u32string u32string;

  typedef std::tuple<u32string, WS*, WS*, WS*> TranslatedEquation;
  typedef std::vector<TranslatedEquation> equation_v;

  typedef boost::uuids::uuid uuid;
  using boost::uuids::nil_uuid;

  class Tuple;

  template <typename T>
  void
  set_constant(Constant& c, T v)
  {
    detail::set_constant_func<T>()(c, v);
  }

  template <typename T>
  T
  get_constant(Constant& c)
  {
    return detail::get_constant_func<T>()(c);
  }

  /**
   * @brief Constant value.
   */
  class Constant
  {
    public:

    Constant()
    {
      data.index = 0;
      data.ptr = nullptr;
    }

    template <typename T>
    Constant(T value, type_index i)
    {
      data.index = i;
      set_constant(*this, value);
    }

    struct
    {
      union
      {
        //Special     sp;
        bool        tv;
        char32_t    ch;
        int8_t      si8;
        uint8_t     ui8;
        int16_t     si16;
        uint16_t    ui16;
        int32_t     si32;
        uint32_t    ui32;
        int64_t     si64;
        uint64_t    ui64;
        float       f32;
        double      f64;
        long double f80;
        void*       ptr;
      };

      type_index index;
      uint16_t field;
    } data;
  };

  typedef size_t dimension_index;
  /**
   * The underlying data structure of a tuple.
   **/
  typedef std::map<dimension_index, Constant> tuple_t;

  /**
   * @brief Stores a Tuple.
   *
   * A tuple is a map from dimension to Constant. Tuple can store
   * unevaluated expressions as a mapped value, TransLucid::TupleIterator
   * will ensure that these are evaluated when their value is required.
   **/
  class Tuple
  {
    public:
    typedef tuple_t::const_iterator const_iterator;
    typedef tuple_t::const_iterator::value_type value_type;

    explicit Tuple(const tuple_t& tuple);
    Tuple();

    Tuple(const Tuple& rhs)
    : m_value(rhs.m_value)
    {
    }

    Tuple* clone() const
    {
      return new Tuple(*this);
    }

    Tuple&
    operator=(const tuple_t& t)
    {
      m_value.reset(new tuple_t(t));
      return *this;
    }

    const_iterator
    begin() const
    {
      return m_value->begin();
    }

    const_iterator
    end() const
    {
      return m_value->end();
    }

    Tuple
    insert(size_t key, const Constant& value) const;

    const_iterator
    find(size_t key) const
    {
      return m_value->find(key);
    }

    const
    tuple_t& tuple() const
    {
      return *m_value;
    }

    size_t
    hash() const
    {
      boost::hash<tuple_t> hasher;
      return hasher(*m_value);
    }

    bool
    operator==(const Tuple& rhs) const
    {
      return *m_value == *rhs.m_value;
    }

    //void
    //print(const System& i, std::ostream& os, const Tuple& c) const;

    void print(std::ostream& os) const;

    Tuple
    copy() const
    {
      return Tuple(new tuple_t(*m_value));
    }

    #if 0
    bool
    operator<(const Tuple& rhs) const
    {
      //return *m_value < *rhs.m_value;
    }
    #endif

    private:

    Tuple(tuple_t* t)
    : m_value(t)
    {}

    boost::shared_ptr<tuple_t> m_value;
  };

  /**
   * @brief A value context pair.
   *
   * A Constant and the context it was evaluated in.
   **/
  typedef std::pair<Constant, Tuple> TaggedConstant;

  //typedef boost::function<Constant
  //        (const Constant&, const Constant&, const Tuple&)> OpFunction;
  //typedef boost::function<Constant
  //        (const Constant&, const Tuple&)> ConvertFunction;

  /**
   * @brief Vector of type indexes.
   **/
  typedef std::vector<type_index> type_vec;


} //namespace TransLucid

#if 0
inline std::ostream&
operator<<(std::ostream& os, const TransLucid::Constant& v)
{
  v.print(os);
  return os;
}

namespace std
{
  template<>
  struct hash<TransLucid::Constant>
  {
    size_t operator()(const TransLucid::Constant& c) const
    {
      return c.hash();
    }
  };
}
#endif

//this has to be included down here so that the definitions are available
//in the right places
#include <tl/detail/types_detail.hpp>

#endif
