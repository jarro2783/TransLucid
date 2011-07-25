/* The main type interface.
   Copyright (C) 2009, 2010, 2011 Jarryd Beck and John Plaice

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

#ifndef TYPES_HPP_INCLUDED
#define TYPES_HPP_INCLUDED

#include <tl/types_fwd.hpp>

#include <boost/functional/hash.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>

#include <iostream>
#include <map>
#include <memory>
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

  enum Special
  {
    SP_ERROR, /**<Error value. Should never have this value, having a special
    of this value means an error occured somewhere.*/
    SP_ACCESS, /**<Access error. Something requested could not be accessed.*/
    SP_TYPEERROR,
    SP_DIMENSION,
    SP_UNDEF,
    SP_CONST,
    SP_MULTIDEF,
    SP_LOOP,
    SPECIAL_LAST //the number of specials, not an actual special value
  };

  template <typename T>
  void
  set_constant(Constant& c, T v)
  {
    detail::set_constant_func<T>()(c, v);
  }

  template <typename T>
  T
  get_constant(const Constant& c)
  {
    return detail::get_constant_func<T>()(c);
  }

  struct TypeFunctions
  {
    bool (*equality)(const Constant&, const Constant&);
    size_t (*hash)(const Constant&);
    void (*destroy)(void*);
  };

  struct ConstantPointerValue
  {
    ConstantPointerValue(TypeFunctions* f, void* d)
    : refCount(1)
    , functions(f)
    , data(d)
    {
    }

    int refCount;
    TypeFunctions* functions;
    void* data;
  };

  enum TypeField
  {
    TYPE_FIELD_ERROR, //not a value, don't read any field
    TYPE_FIELD_SP, 
    TYPE_FIELD_TV, //truth value
    TYPE_FIELD_CH,
    TYPE_FIELD_SI8,
    TYPE_FIELD_UI8,
    TYPE_FIELD_SI16,
    TYPE_FIELD_UI16,
    TYPE_FIELD_SI32,
    TYPE_FIELD_UI32,
    TYPE_FIELD_SI64,
    TYPE_FIELD_UI64,
    TYPE_FIELD_F32,
    TYPE_FIELD_F64,
    TYPE_FIELD_PTR
  };

  /**
   * @brief Constant value.
   */
  class Constant
  {
    public:

    Constant()
    {
      data.ptr = nullptr;
      data.index = 0;
      data.field = TYPE_FIELD_ERROR;
    }

    ~Constant()
    {
      if (data.field == TYPE_FIELD_PTR)
      {
        removeReference();    
      }
    }

    Constant(Constant&& other)
    {
      copyData(other);
      other.data.ptr = nullptr;
      other.data.index = 0;
      other.data.field = TYPE_FIELD_ERROR;
    }

    Constant(ConstantPointerValue* p, type_index i)
    {
      data.ptr = p;
      data.field = TYPE_FIELD_PTR;
      data.index = i;
    }

    Constant(const Constant& other)
    {
      copyData(other);
      if (other.data.field == TYPE_FIELD_PTR)
      {
        increaseReference();
      }
    }

    Constant&
    operator=(const Constant& rhs)
    {
      if (this != &rhs)
      {
        //nothrow assignment
        if (data.field == TYPE_FIELD_PTR)
        {
          removeReference();
        }

        copyData(rhs);

        if (data.field == TYPE_FIELD_PTR)
        {
          increaseReference();
        }
      }
      return *this;
    }

    template <typename T>
    Constant(T value, type_index i)
    {
      data.index = i;
      set_constant(*this, value);
    }

    size_t
    hash() const;

    bool
    operator==(const Constant& rhs) const
    {
      if (data.index == rhs.data.index)
      {
        if (data.field < TYPE_FIELD_PTR)
        {
          return detail::constant_equality(*this, rhs);
        }
        else
        {
          return (*data.ptr->functions->equality)(*this, rhs);
        }
      }
      else
      {
        return false;
      }
    }

    type_index
    index() const
    {
      return data.index;
    }

    struct
    {
      union
      {
        Special     sp;
        bool        tv; //truth value
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

        ConstantPointerValue* ptr;
      };

      type_index index;
      TypeField field;
    } data;

    private:

    void
    removeReference()
    {
      --data.ptr->refCount;

      if (data.ptr->refCount == 0)
      {
        (*data.ptr->functions->destroy)(data.ptr->data);
        delete data.ptr;
      }
    }

    void 
    increaseReference()
    {
      ++data.ptr->refCount;
    }

    void
    copyData(const Constant& other)
    {
      memcpy(&data, &other.data, sizeof(data));
    }
  };

  inline size_t
  hash_value(const Constant& c)
  {
    return c.hash();
  }

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

    Tuple(tuple_t&& rhs)
    : m_value(new tuple_t(std::move(rhs)))
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

    //perturb the current tuple and return a new one
    Tuple
    at(const tuple_t& k) const;

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

    std::shared_ptr<tuple_t> m_value;
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
#endif

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

//this has to be included down here so that the definitions are available
//in the right places
#include <tl/detail/types_detail.hpp>

#endif
