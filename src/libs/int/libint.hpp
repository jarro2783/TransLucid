/* Integer library.
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

#ifndef INTLIB_TYPEMANAGER_HPP_INCLUDED
#define INTLIB_TYPEMANAGER_HPP_INCLUDED

#include <tl/library.hpp>
#include <tl/types.hpp>
#include <boost/lexical_cast.hpp>

namespace IntLib
{

  template <class T>
  class Int : public TransLucid::TypedValue
  {
    public:
    Int(T value)
    : m_value(value)
    {}

    Int(const Int& rhs)
    : m_value(rhs.m_value)
    {
    }

    Int* clone() const
    {
      return new Int<T>(*this);
    }

    void
    print(std::ostream& os) const;

    bool
    operator==(const Int& rhs) const
    {
      return m_value == rhs.m_value;
    }

    const T&
    value() const
    {
      return m_value;
    }

    //static Int<T>
    //parse
    //(
    //  const TransLucid::u32string& text,
    //  const TransLucid::Tuple& k,
    //  const TransLucid::System& i
    //)
    //{
    //  return Int<T>(boost::lexical_cast<T>(text));
    //}

    size_t
    hash() const
    {
      boost::hash<T> hasher;
      return hasher(m_value);
    }

    bool
    operator<(const Int<T>& rhs) const
    {
      return m_value < rhs.m_value;
    }

    private:
    T m_value;
  };

  template <class T>
  size_t
  hash_value(const Int<T>& v)
  {
    boost::hash<T> hasher;
    return hasher(v.value());
  }
};

#endif // TYPEMANAGER_HPP_INCLUDED
