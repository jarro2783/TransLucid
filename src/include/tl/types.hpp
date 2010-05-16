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
// before the interpreter class is then this
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

  class Interpreter;

  class HD;

  //class EquationGuard;

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

    //puts the T& into a flyweight
    //T has to extend TypedValue
    /**
     * @brief Construct a typed value.
     *
     * The type is referenced by @a index which must be a valid index
     * in TransLucid::TypeRegistry. T must inherit from TypeValueBase.
     **/
    template <typename T>
    Constant(const T& value, type_index index)
    : m_value(new Storage<T>(value)), m_index(index)
    {
    }

    Constant(const Constant& other)
    : m_value(other.m_value != 0 ? other.m_value->clone() : 0),
      m_index(other.m_index)
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
        //if this throws then leave the value as it is, having
        //a 0 here would be bad since Constant should always have
        //a value, so it's better to leave the old value
        StorageBase* copy = rhs.m_value->clone();
        delete m_value;
        m_value = copy;
        m_index = rhs.m_index;
      }

      return *this;
    }

    bool
    operator==(const Constant& rhs) const
    {
      return m_index == rhs.m_index
      && m_value->equalTo(*rhs.m_value);
    }

    bool
    operator!=(const Constant& rhs) const
    {
      return !(*this == rhs);
    }

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

    /**
     * @brief Computes the hash of the typed value.
     *
     * The hash includes the index.
     **/
    size_t
    hash() const
    {
      size_t seed = m_index;
      boost::hash_combine(seed, m_value->value().hash());
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
      return dynamic_cast<const T&>(m_value->value());
    }

    /**
     * @brief Retrieve the value stored.
     *
     * Returns the value stored as a TypedValue reference.
     **/
    const TypedValue&
    value() const
    {
      return m_value->value();
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
      return dynamic_cast<const T*>(&m_value->value());
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

    //allows us to store values of any type
    class StorageBase
    {
      public:
      virtual ~StorageBase() {}
      virtual const TypedValue& value() const = 0;
      virtual StorageBase* clone() const = 0;
      virtual bool equalTo(const StorageBase& other) const = 0;
      virtual bool less(const StorageBase& other) const = 0;
      virtual void print(std::ostream& os) const = 0;
    };

    //stores a value of type T
    template <class T>
    class Storage : public StorageBase
    {
      public:
      Storage(const T& v)
      : m_value(v)
      {
      }

      Storage(const Storage& other)
      : m_value(other.m_value)
      {
      }

      const TypedValue&
      value() const
      {
        return m_value;
      }

      Storage<T>*
      clone() const
      {
        return new Storage<T>(*this);
      }

      bool
      equalTo(const StorageBase& other) const
      {
        const Storage<T>* o = dynamic_cast<const Storage<T>*>(&other);
        if (o)
        {
          return m_value == o->m_value;
        }
        else
        {
          return false;
        }
      }

      bool
      less(const StorageBase& other) const
      {
        const Storage<T>* o = dynamic_cast<const Storage<T>*>(&other);
        if (o)
        {
          return m_value < o->m_value;
        }
        else
        {
          throw "tried to compare less than for two values of different types";
        }
      }

      void
      print(std::ostream& os) const
      {
        m_value.print(os);
      }

      private:
      T m_value;
    };

    StorageBase* m_value;
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
    //print(const Interpreter& i, std::ostream& os, const Tuple& c) const;

    void print(std::ostream& os) const;

    Tuple
    copy() const
    {
      return Tuple(new tuple_t(*m_value));
    }

    bool
    operator<(const Tuple& rhs) const
    {
      return *m_value < *rhs.m_value;
    }

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
  typedef std::pair<Constant, Tuple> TaggedValue;

  //typedef boost::function<Constant
  //        (const Constant&, const Constant&, const Tuple&)> OpFunction;
  //typedef boost::function<Constant
  //        (const Constant&, const Tuple&)> ConvertFunction;

  /**
   * @brief Vector of type indexes.
   **/
  typedef std::vector<type_index> type_vec;

  #if 0

  //keeps track of types
  /**
   * @brief Stores types and operations on a types.
   *
   * Stores all the types in a system and their associated printer, parser
   * and operations.
   **/
  class TypeRegistry
  {
    public:

    //the registry is tied to a specific interpreter
    TypeRegistry(Interpreter& i);
    ~TypeRegistry();

    type_index
    generateIndex()
    {
      return m_nextIndex++;
    }

    size_t
    registerType(const u32string& name);

    size_t
    lookupType(const u32string& name) const;

    #if 0
    void
    registerOp
    (const Glib::ustring& name, const type_vec& operands, OpFunction op)
    {
      op_map& ops = opsMap[name];
      ops[operands] = op;
    }

    OpFunction
    findOp(const Glib::ustring& name, const type_vec& tv) const
    {
      ops_map::const_iterator opsit = opsMap.find(name);
      if (opsit == opsMap.end())
      {
        return makeOpTypeError;
      }
      op_map::const_iterator opit = opsit->second.find(tv);
      if (opit == opsit->second.end())
      {
        return makeOpTypeError;
      }
      return opit->second;
    }

    //a variadic op takes a list of operands all of the same type
    void
    registerVariadicOp(const u32string& name, type_index t, OpFunction op)
    {
      m_variadicOperators[name][t] = op;
    }

    OpFunction
    findVariadicOp(const Glib::ustring& name, type_index t) const
    {
      VariadicOpMap::const_iterator opsit = m_variadicOperators.find(name);
      if (opsit == m_variadicOperators.end())
      {
        return makeOpTypeError;
      }
      VariadicOpMap::mapped_type::const_iterator opit =
        opsit->second.find(t);
      if (opit == opsit->second.end())
      {
        return makeOpTypeError;
      }
      return opit->second;
    }
    #endif

    Interpreter&
    interpreter() const
    {
      return m_interpreter;
    }

    //indexes for built in types
    type_index
    indexSpecial() const
    {
      return m_indexSpecial;
    }

    type_index
    indexBoolean() const
    {
      return m_indexBool;
    }

    type_index
    indexTuple() const
    {
      return m_indexTuple;
    }

    type_index
    indexUneval() const
    {
      return m_indexUneval;
    }

    type_index
    indexIntmp() const
    {
      return m_indexIntmp;
    }

    type_index
    indexDimension() const
    {
      return m_indexDimension;
    }

    type_index
    indexRange() const
    {
      return m_indexRange;
    }

    type_index
    indexExpr() const
    {
      return m_indexExpr;
    }

    type_index
    indexString() const
    {
      return m_indexString;
    }

    type_index
    indexCalc() const
    {
      return m_indexCalc;
    }

    type_index
    indexGuard() const
    {
      return m_indexGuard;
    }

    type_index
    indexPair() const
    {
      return m_indexPair;
    }

    type_index
    indexChar() const
    {
      return m_indexChar;
    }

    private:

    type_index m_nextIndex;

    type_index m_indexError;
    type_index m_indexSpecial;
    type_index m_indexBool;
    type_index m_indexTuple;
    type_index m_indexUneval;
    type_index m_indexIntmp;
    type_index m_indexDimension;
    type_index m_indexRange;
    type_index m_indexExpr;
    type_index m_indexString;
    type_index m_indexChar;
    type_index m_indexCalc;
    type_index m_indexGuard;
    type_index m_indexPair;

    Interpreter& m_interpreter;

    public:
    #if 0
    ConvertFunction
    findConverter(type_index to, type_index from) const
    {
      ConvertorsMap::const_iterator iter = m_convertors.find(to);

      if (iter == m_convertors.end())
      {
        return makeConvertTypeError;
      }

      IndexConvertMap::const_iterator iter2 = iter->second.find(from);
      if (iter2 == iter->second.end())
      {
        return makeConvertTypeError;
      }

      return iter2->second;
    }
    #endif
  };
  #endif

} //namespace TransLucid

inline std::ostream&
operator<<(std::ostream& os, const TransLucid::Constant& v)
{
  v.print(os);
  return os;
}

#endif
