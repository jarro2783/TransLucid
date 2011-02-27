/* Utility functions.
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

#ifndef TL_UTILITY_HPP_INCLUDED
#define TL_UTILITY_HPP_INCLUDED

#include <tl/builtin_types.hpp>
#include <tl/fixed_indexes.hpp>
#include <tl/hyperdaton.hpp>
#include <tl/range.hpp>
#include <tl/types.hpp>

#include <vector>

#include <boost/algorithm/string/find.hpp>
#include <boost/algorithm/string/classification.hpp>

namespace TransLucid
{
  class DimensionNotFound
  {
  };

  inline Constant
  makeTime(const mpz_class& time)
  {
    return Constant(Intmp(time), TYPE_INDEX_INTMP);
  }

  class SplitID
  {
    private:
    static const u32string m_split_match;;

    u32string::const_iterator m_begin;
    u32string::const_iterator m_end;
    const u32string& m_s;

    public:
    SplitID(const u32string& s)
    : m_s(s)
    {
      boost::iterator_range<u32string::const_iterator> r =
        boost::algorithm::find_first(s, m_split_match);
      m_begin = r.begin();
      m_end = r.end();
    }

    bool
    has_components()
    {
      return m_begin != m_end;
    }

    u32string
    first()
    {
      return u32string(m_s.begin(), m_begin);
    }

    u32string
    last()
    {
      return u32string(m_end, m_s.end());
    }
  };

  inline Constant
  generate_string(const u32string& name)
  {
    return Constant(String(name), TYPE_INDEX_USTRING);
  }

  inline mpz_class
  get_type_index(HD* h, const u32string& name)
  {
    tuple_t k;
    k[DIM_ID] = generate_string(U"TYPE_INDEX");
    //Constant(String("TYPE_INDEX"), TYPE_INDEX_USTRING);
    k[DIM_TYPE] = generate_string(name);
    //Constant(String(name), TYPE_INDEX_USTRING);
    return (*h)(Tuple(k)).first.value<Intmp>().value();
  }

  //pre: the base is valid, if not we will return 10
  //but don't rely on this for error checking
  template <typename Char>
  int get_numeric_base(Char c)
  {
    if (c >= '0' && c <= '9')
    {
      return c - '0';
    }
    else if (c >= 'A' && c <= 'Z')
    {
      return c - 'A' + 10;
    }
    else if (c >= 'a' && c <= 'z')
    {
      return c - 'a' + 10 + 26;
    }
    return 10;
  }

  inline mpz_class
  get_unique(HD* h)
  {
    tuple_t k;
    k[DIM_ID] = Constant(String(U"_unique"), TYPE_INDEX_USTRING);
    return (*h)(Tuple(k)).first.value<Intmp>().value();
  }

  inline size_t
  get_dimension_index(HD* h, const u32string& name)
  {
    tuple_t k;
    k[DIM_ID] = generate_string(U"DIMENSION_NAMED_INDEX");
    k[DIM_TEXT] = generate_string(name);
    return (*h)(Tuple(k)).first.value<Intmp>().value().get_ui();
  }

  inline size_t
  get_dimension_index(HD* h, const Constant& v)
  {
    tuple_t k;
    k[DIM_ID] = generate_string(U"DIMENSION_VALUE_INDEX");
    k[DIM_VALUE] = v;
    return (*h)(Tuple(k)).first.value<Intmp>().value().get_ui();
  }

  inline Constant
  get_dimension(const Tuple& k, size_t index)
  {
    Tuple::const_iterator i = k.find(index);
    if (i == k.end())
    {
      throw DimensionNotFound();
    }
    return i->second;
  }

  template <typename T>
  class FunctorHD : public HD
  {
    public:
    FunctorHD(const T& f)
    : m_f(f)
    {}

    TaggedConstant
    operator()(const Tuple& k)
    {
      return m_f(k);
    }

    void
    addExpr(const Tuple& k, HD* h)
    {
    }

    private:
    T m_f;
  };

  template <typename T>
  FunctorHD<T>* generate_functor_hd(const T& f)
  {
    return new FunctorHD<T>(f);
  }

  bool
  tupleApplicable(const Tuple& def, const Tuple& c);

  bool
  tupleRefines(const Tuple& a, const Tuple& b);

  bool
  valueRefines(const Constant& a, const Constant& b);

  bool
  booleanTrue(const GuardHD& g, const Tuple& c);

  tuple_t
  create_add_eqn_context
  (
    const u32string& name,
    HD* guard,
    HD* boolean,
    const mpz_class& time
  );

  //looks up a value in the current context and returns the value of the
  //all dimension if it exists, otherwise special<dim> if not found
  TaggedConstant
  lookup_context(HD* system, const Constant& v, const Tuple& k);

  inline Constant
  make_special(Special::Value s)
  {
    return Constant(Special(s), TYPE_INDEX_SPECIAL);
  }

  //returns the hash of a dimension when we only have the index
  class HashIndexHD : public HD
  {
    public:
    HashIndexHD(dimension_index index)
    : m_index(index)
    {
    }

    TaggedConstant
    operator()(const Tuple& k)
    {
      Tuple::const_iterator iter = k.find(m_index);
      if (iter != k.end())
      {
        return TaggedConstant(iter->second, k);
      }
      else
      {
        return TaggedConstant(make_special(Special::DIMENSION), k);
      }
    }

    private:
    dimension_index m_index;
  };
}

#endif // TL_UTILITY_HPP_INCLUDED
