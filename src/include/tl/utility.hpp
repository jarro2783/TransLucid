/* Utility functions.
   Copyright (C) 2009--2012 Jarryd Beck

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

/** @file utility.hpp
 * Random utility stuff.
 */

#ifndef TL_UTILITY_HPP_INCLUDED
#define TL_UTILITY_HPP_INCLUDED

#include <tl/builtin_types.hpp>
#include <tl/equation.hpp>
#include <tl/fixed_indexes.hpp>
#include <tl/workshop.hpp>
#include <tl/range.hpp>
#include <tl/types.hpp>
#include <tl/types/string.hpp>
#include <tl/types/special.hpp>

#include <vector>

namespace TransLucid
{
  class DimensionNotFound
  {
  };

  inline Constant
  generate_string(const u32string& name)
  {
    //return Constant(String(name), TYPE_INDEX_USTRING);
    return Types::String::create(name);
  }

  #if 0
  inline mpz_class
  get_type_index(WS* h, const u32string& name)
  {
    tuple_t k;
    k[DIM_ID] = generate_string(U"TYPE_INDEX");
    //Constant(String("TYPE_INDEX"), TYPE_INDEX_USTRING);
    k[DIM_TYPE] = generate_string(name);
    //Constant(String(name), TYPE_INDEX_USTRING);
    return (*h)(Tuple(k)).first.value<Intmp>().value();
  }
  #endif

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
  class FunctorWS : public WS
  {
    public:
    FunctorWS(const T& f)
    : m_f(f)
    {}

    TaggedConstant
    operator()(const Tuple& k)
    {
      return m_f(k);
    }

    void
    addExpr(const Tuple& k, WS* h)
    {
    }

    private:
    T m_f;
  };

  template <typename T>
  FunctorWS<T>* generate_functor_hd(const T& f)
  {
    return new FunctorWS<T>(f);
  }

  bool
  tupleApplicable(const Tuple& def, const Context& c);

  bool
  tupleRefines(const Tuple& a, const Tuple& b, bool canequal = false);

  bool
  valueRefines(const Constant& a, const Constant& b);

  bool
  booleanTrue(const EquationGuard& g, Context& c);

  //the cached boolean true
  bool
  booleanTrue
  (
    const EquationGuard& g, 
    Context& c, 
    Context& delta, 
    std::vector<dimension_index>& demands
  );

  //looks up a value in the current context
  Constant
  lookup_context(System& system, const Constant& v, const Context& k);

  //looks up a value in the current context, respecting the cache rules
  Constant
  lookup_context_cached
  (
    System& system, 
    const Constant& v, 
    const Context& delta
  );

  //returns the hash of a dimension when we only have the index
  class HashIndexWS : public WS
  {
    public:
    HashIndexWS(dimension_index index)
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
        return TaggedConstant(Types::Special::create(SP_DIMENSION), k);
      }
    }

    private:
    dimension_index m_index;
  };

  Tuple
  makeList(const Constant& c, const Constant& tail);

  Constant
  listHead(const Constant& l);

  Constant
  listTail(const Constant& l);

  std::string
  read_file(std::istream& is);

  Constant
  applyFunction(Context& k, const Constant& lhs, const Constant& rhs);

  Constant
  applyFunction
  (
    Context& kappa, 
    Context& delta, 
    const Constant& lhs, 
    const Constant& rhs
  );

  struct ExtraTreeInformation
  {
    std::vector<std::pair<ScopePtr, Parser::Line>> equations;
  };

  std::pair
  <
    Tree::Expr,
    ExtraTreeInformation
  >
  fixupTree(System& s, const Tree::Expr& e, ScopePtr scope = ScopePtr());
}

#endif // TL_UTILITY_HPP_INCLUDED
