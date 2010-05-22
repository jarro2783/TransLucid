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

#include <iostream>

#include <boost/functional/hash.hpp>
#include <map>
#include <boost/unordered_map.hpp>
#include <vector>
#include <boost/shared_ptr.hpp>
#include <gmpxx.h>
#include <boost/foreach.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>

#define STRING(x) #x
#define STRING_(x) STRING(x)
//#include <tl/exception.hpp>

// ----------------------------------
// WARNING !!!
// Caveat with using boost flyweight
// all Constants must be destructed
// before unloading libraries with
// lt_exit
// as long as all typed values are destructed
// before the system class is then this
// will work fine
// ----------------------------------

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
      BOOST_FOREACH(int i, id)
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

  class SystemHD;

  class HD;

  //class GuardHD;

  typedef uint16_t type_index;
  typedef std::u32string u32string;

  typedef std::tuple<u32string, HD*, HD*, HD*> TranslatedEquation;
  typedef std::vector<TranslatedEquation> equation_v;

  typedef boost::uuids::uuid uuid;
  using boost::uuids::nil_uuid;

  class Tuple;

  /**
   * @brief Base class for typed value data.
   *
   * The actual representation for typed values are derived from this.
   **/
  class TypedValue
  {
    public:
    virtual ~TypedValue() {}

    virtual TypedValue* clone() const = 0;

    /**
     * @brief Compute the hash of a typed value.
     **/
    virtual size_t hash() const = 0;

    virtual void print(std::ostream& os) const = 0;
  };

  /**
   * @brief Computes the hash of a TypedValue.
   **/
  inline size_t
  hash_value(const TypedValue& v)
  {
    return v.hash();
  }

  /**
   * @brief Cooked value holder.
   *
   * Stores a cooked type value pair.
   **/
  class Constant
  {
    public:

    //error type
    /**
     * @brief Constructs an error value.
     *
     * This represents the internal error value. Nothing should ever
     * have this value.
     **/
    Constant()
    : m_value(0), m_index(0)
    {
    }

    /**
     * @brief Construct a typed value.
     *
     * The type is referenced by @a index which must be a valid index
     * in TransLucid::TypeRegistry. T must inherit from TypedValue.
     **/
    template <typename T>
    Constant(const T& value, type_index index)
    : m_value(new T(value)),
    m_index(index)
    {
    }

    Constant(const Constant& rhs)
    : 
    m_value(rhs.m_value != 0 ? rhs.m_value->clone() : 0),
    m_index(rhs.m_index)
    {
    }

    ~Constant() {
      delete m_value;
    }

    Constant&
    operator=(const Constant& rhs)
    {
      if (this != &rhs)
      {
        //if this throws nothing else happens, the object stays as it is
        //and the exception goes on
        TypedValue* v = rhs.m_value->clone();
        delete m_value;
        m_value = v;
        m_index = rhs.m_index;
      }

      return *this;
    }

    bool
    operator==(const Constant& rhs) const
    {
      return m_index == rhs.m_index
      && m_value->hash() == rhs.m_value->hash();
    }

    bool
    operator!=(const Constant& rhs) const
    {
      return !(*this == rhs);
    }

    #if 0
    bool
    operator<(const Constant& rhs) const
    {
      if (m_index != rhs.m_index)
      {
        return m_index < rhs.m_index;
      }
      else
      {
        return m_value->less(*rhs.m_value);
      }
    }
    #endif

    /**
     * @brief Computes the hash of the typed value.
     *
     * The hash includes the index.
     **/
    size_t
    hash() const
    {
      size_t seed = m_index;
      boost::hash_combine(seed, m_value->hash());
      return seed;
    }

    //get the value stored, provides a function that returns the actual
    //value and a reference and pointer function that do the dynamic
    //cast for the user
    /**
     * @brief Retrieve the value stored.
     *
     * Attempts to cast the value stored to T.
     * @throw bad_cast if the cast fails.
     **/
    template <typename T>
    const T&
    value() const
    {
      return dynamic_cast<const T&>(*m_value);
    }

    /**
     * @brief Retrieve the value stored.
     *
     * Returns the value stored as a TypedValue reference.
     **/
    const TypedValue&
    value() const
    {
      return *m_value;
    }

    /**
     * @brief Retrieve the value stored.
     *
     * Attempts to cast the value stored to T, if it fails 0 is returned.
     **/
    template <typename T>
    const T*
    valuep() const
    {
      return dynamic_cast<const T*>(m_value);
    }

    /**
     * @brief Get the type index.
     *
     * @return The type index this was constructed with.
     **/
    type_index
    index() const
    {
      return m_index;
    }

    void
    print(std::ostream& os) const
    {
      m_value->print(os);
    }

    private:

    TypedValue* m_value;
    type_index m_index;
  };

  /**
   * @brief Computes the hash of a Constant.
   **/
  inline size_t
  hash_value(const Constant& value)
  {
    return value.hash();
  }

  /**
   * The underlying data structure of a tuple.
   **/
  typedef std::map<size_t, Constant> tuple_t;

  /**
   * @brief Stores a Tuple.
   *
   * A tuple is a map from dimension to Constant. Tuple can store
   * unevaluated expressions as a mapped value, TransLucid::TupleIterator
   * will ensure that these are evaluated when their value is required.
   **/
  class Tuple : public TypedValue
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
    //print(const SystemHD& i, std::ostream& os, const Tuple& c) const;

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

inline std::ostream&
operator<<(std::ostream& os, const TransLucid::Constant& v)
{
  v.print(os);
  return os;
}

#endif
