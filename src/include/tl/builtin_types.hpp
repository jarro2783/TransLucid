/* Built-in types.
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

#ifndef BUILTIN_TYPES_HPP_INCLUDED
#define BUILTIN_TYPES_HPP_INCLUDED

#include <tl/types.hpp>
#include <tl/equation.hpp>
#include <set>

namespace TransLucid
{
  class Special : public TypedValueBase
  {
    public:
    enum Value
    {
      ERROR,
      ACCESS,
      TYPEERROR,
      DIMENSION,
      UNDEF,
      CONST,
      MULTIDEF,
      LOOP
    };

    Special(const u32string& text)
    : m_v(stringToValue(text))
    {}

    Special(Value v)
    : m_v(v)
    {}

    const Value
    value() const
    {
      return m_v;
    }

    void
    print(std::ostream& os) const;

    bool
    operator==(const Special& rhs) const
    {
      return m_v == rhs.m_v;
    }

    size_t
    hash() const
    {
      return m_v;
    }

    bool
    operator<(const Special& rhs) const
    {
      return m_v < rhs.m_v;
    }

    private:
    Value m_v;

    struct StringValueInitialiser
    {
      typedef boost::unordered_map<u32string, Value> StringValueMap;
      StringValueMap stov;

      typedef boost::unordered_map<Value, u32string> ValueStringMap;
      ValueStringMap vtos;

      StringValueInitialiser();
    };

    static StringValueInitialiser m_sv;

    public:

    static Value
    stringToValue(const u32string& s)
    {
      StringValueInitialiser::StringValueMap::const_iterator iter
        = m_sv.stov.find(s);
      if (iter == m_sv.stov.end())
      {
        return ERROR;
      }
      else
      {
        return iter->second;
      }
    }
  };

  class String : public TypedValueBase
  {
    public:
    String(const u32string& s)
    : m_s(s)
    {
    }

    size_t
    hash() const
    {
      boost::hash<u32string> hasher;
      return hasher(m_s);
    }

    static String
    parse(const u32string& text)
    {
      return String(text);
    }

    void
    print(std::ostream& os) const;

    bool
    operator==(const String& rhs) const
    {
      return m_s == rhs.m_s;
    }

    bool
    operator<(const String& rhs) const
    {
      return m_s < rhs.m_s;
    }

    const u32string&
    value() const
    {
      return m_s;
    }

    private:
    u32string m_s;
  };

  class Boolean : public TypedValueBase
  {
    public:

    Boolean(bool v)
    : m_value(v)
    {
    }

    operator bool() const
    {
      return m_value;
    }

    size_t
    hash() const
    {
      return m_value ? 1 : 0;
    }

    void
    print(std::ostream& os) const
    {
      if (m_value)
      {
        os << "true";
      }
      else
      {
        os << "false";
      }
    }

    private:
    bool m_value;
  };

  class Intmp : public TypedValueBase
  {
    public:
    Intmp(const mpz_class& value)
    : m_value(value)
    {
    }

    size_t
    hash() const
    {
      boost::hash<mpz_class> hasher;
      return hasher(m_value);
    }

    void
    print(std::ostream& os) const
    {
      os << "intmp<" << m_value.get_str() << ">";
    }

    bool
    operator==(const Intmp& rhs) const
    {
      return m_value == rhs.m_value;
    }

    const mpz_class&
    value() const
    {
      return m_value;
    }

    bool
    operator<(const Intmp& rhs) const
    {
      return m_value < rhs.m_value;
    }

    private:
    mpz_class m_value;
  };

  class Dimension : public TypedValueBase
  {
    public:
    Dimension(size_t value)
    : m_value(value)
    {}

    size_t
    value() const
    {
      return m_value;
    }

    size_t
    hash() const
    {
      return m_value;
    }

    void
    print(std::ostream& os) const
    {
      os << "dimension: '" << m_value << "'";
    }

    bool
    operator==(const Dimension& rhs) const
    {
      return m_value == rhs.m_value;
    }

    bool
    operator<(const Dimension& rhs) const
    {
      return m_value < rhs.m_value;
    }

    private:
    size_t m_value;
  };

  typedef std::set<TypedValue> set_t;

  class Set : public TypedValueBase
  {
    public:
    Set();

    bool
    operator==(const Set& rhs) const;

    void
    print(std::ostream& os, const Tuple& c) const;

    size_t
    hash() const;

    const set_t&
    value() const;
  };

  class ValueCalc : public TypedValueBase
  {
    public:
    size_t
    hash() const
    {
      return 0;
    }

    bool
    operator==(const ValueCalc& rhs) const
    {
      return true;
    }

    bool
    operator<(const ValueCalc& rhs) const
    {
      return false;
    }
  };

  class Char : public TypedValueBase
  {
    public:
    Char(char32_t c)
    : m_c(c)
    {
    }

    char32_t
    value() const
    {
      return m_c;
    }

    size_t
    hash() const
    {
      return m_c;
    }

    void
    print(std::ostream& os) const;

    bool
    operator==(const Char& rhs) const
    {
      return m_c == rhs.m_c;
    }

    bool
    operator<(const Char& rhs) const
    {
      return m_c < rhs.m_c;
    }

    private:
    char32_t m_c;
  };

  class EquationGuardType : public TypedValueBase
  {
    public:
    EquationGuardType(const EquationGuard& g)
    : m_g(g)
    {}

    const EquationGuard&
    value() const
    {
      return m_g;
    }

    bool
    operator==(const EquationGuardType& rhs) const
    {
      return true;
    }

    bool
    operator<(const EquationGuardType& rhs) const
    {
      return false;
    }

    size_t
    hash() const
    {
      return 0;
    }

    void
    print(std::ostream& os) const
    {
      os << "equation guard";
    }

    private:
    EquationGuard m_g;
  };

  class PairType : public TypedValueBase
  {
    public:
    PairType(const TypedValue& first, const TypedValue& second)
    : m_first(first), m_second(second)
    {
    }

    const TypedValue&
    first() const
    {
      return m_first;
    }

    const TypedValue&
    second() const
    {
      return m_second;
    }

    size_t
    hash() const
    {
      return 0;
    }

    bool
    operator==(const PairType&) const
    {
      return true;
    }

    bool
    operator<(const PairType&) const
    {
      return false;
    }

    void print(std::ostream& os) const
    {
      os << m_first << " : " << m_second;
    }

    private:
    TypedValue m_first;
    TypedValue m_second;
  };

  class SetBase
  {
    public:
    //is v a member of this
    virtual bool
    is_member(const TypedValue& v) = 0;

    //is s a subset of this
    virtual bool
    is_subset(const SetBase& s) = 0;
  };

  //the general set type
  //all the actual sets will put a derived class in here and then
  //have an is member function.
  class SetType : public TypedValueBase
  {
    public:

    SetType(SetBase* v)
    : m_value(v)
    {}

    bool
    operator==(const SetType& rhs) const
    {
      return m_value == rhs.m_value;
    }

    bool
    operator<(const SetType& rhs) const
    {
      return m_value < rhs.m_value;
    }

    size_t
    hash() const
    {
      return reinterpret_cast<size_t>(m_value);
    }

    private:
    SetBase* m_value;
  };

  class TypeType : public TypedValueBase
  {
    public:

    TypeType(size_t index)
    : m_index(index)
    {}

    bool
    operator==(const TypeType& rhs) const
    {
      return m_index == rhs.m_index;
    }

    bool
    operator<(const TypeType& rhs) const
    {
      return m_index < rhs.m_index;
    }

    void
    print(std::ostream& os) const
    {
      os << "type<" << m_index << ">";
    }

    size_t
    hash() const
    {
      return m_index;
    }

    size_t
    index() const
    {
      return m_index;
    }

    private:
    size_t m_index;
  };
}

#endif // BUILTIN_TYPES_HPP_INCLUDED
