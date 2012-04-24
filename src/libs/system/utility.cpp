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

/** @file utility.cpp
 * Random utility functions that don't really belong anywhere.
 */

//iconv is being silly, so we'll see what happens with this
//possibly it was just a stack overflow from something else
//#define HAND_CONVERT
#define ICONV_CONVERT

#include <iostream>

#include <tl/utility.hpp>
#include <tl/equation.hpp>
#include <tl/system.hpp>
#include <tl/types.hpp>
#include <tl/types/demand.hpp>
#include <tl/types/function.hpp>
#include <tl/types/intmp.hpp>
#include <tl/types/range.hpp>
#include <tl/types/special.hpp>
#include <tl/types/tuple.hpp>
#include <tl/types/type.hpp>
#include <tl/types/union.hpp>

#ifdef ICONV_CONVERT
#include <iconv.h>
#endif

//the subset functions are subseteq

namespace TransLucid
{

namespace
{
  //is a a subset of b
  typedef bool (*IsSubsetFn)(const Constant& a, const Constant& b);  

  //returns a function that determines if the parameter is a subset of
  //the particular type that it handles
  typedef IsSubsetFn (*IsSubsetOf)(const Constant&);

  //several things are immediately false
  bool
  issubset_false(const Constant&, const Constant&)
  {
    return false;
  }

  bool
  constants_equal(const Constant& a, const Constant& b)
  {
    return a == b;
  }

  //there are several types that can only be compared with something
  //of the same type, instead of writing the code several times for
  //each, we will use templates.
  //In that case, constant equality works fine.
  template <type_index T>
  IsSubsetFn
  isSubsetAtomic(const Constant& c)
  {
    if (c.index() != T)
    {
      return &issubset_false;
    }
    else
    {
      return &constants_equal;
    }
  }

  //TODO: work out tuples and tuple sets
  //this currently returns false if the two are equal
  //we probably want it to be true, but in some cases we want it to
  //be false, so we have to work out what it all means
  bool
  tuple_subset_tuple(const Constant& sub, const Constant& super)
  {
    //std::cerr << "tuple subset in bestfit :)" << std::endl;
    //bool r = 
    return 
    tupleRefines
    (
      Types::Tuple::get(sub),
      Types::Tuple::get(super),
      true
    );

    //std::cerr << "refines = " << r << std::endl;
    //return r;
  }

  IsSubsetFn 
  subset_of_tuple(const Constant& c)
  {
    if (c.index() != TYPE_INDEX_TUPLE)
    {
      return &issubset_false;
    }
    else
    {
      return &tuple_subset_tuple;
    }
  }

  IsSubsetFn
  subset_of_range(const Constant& c)
  {
    //at the moment we can only do ranges of integers
    
    if (c.index() == TYPE_INDEX_INTMP)
    {
      //lambda functions to the rescue
      return [] (const Constant& a, const Constant& b)
        {
          //a is an int, b is a range
          return Types::Range::get(b).within(Types::Intmp::get(a));
        }
      ;
    }
    else if (c.index() == TYPE_INDEX_RANGE)
    {
      return [] (const Constant& a, const Constant& b)
        {
          //a is a range, b is a range
          return Types::Range::get(b).within(Types::Range::get(a));
        }
      ;
    }
    else
    {
      return &issubset_false;
    }
  }

  IsSubsetFn
  subset_of_type(const Constant& c)
  {
    return [] (const Constant& a, const Constant& b)
      {
        if (a.index() == TYPE_INDEX_TYPE)
        {
          return a == b;
        }
        //b is a type, a is anything
        return (a.index() == get_constant<type_index>(b));
      }
    ;
  }

  bool subset_union_constant(const Constant& a, const Constant& b)
  {
    //a is a union
    const UnionType& u = Types::Union::get(b);

    return u.contains(a);
  }

  bool subset_union_union(const Constant& a, const Constant& b)
  {
    return false;
  }

  IsSubsetFn
  subset_of_union(const Constant& c)
  {
    if (c.index() == TYPE_INDEX_UNION)
    {
      return &subset_union_union;
    }
    else
    {
      return &subset_union_constant;
    }
  }

  class TypeComparators
  {
    public:

    TypeComparators()
    {
      m_funs = new IsSubsetOf[NUM_FUNS]
        {
          &isSubsetAtomic<TYPE_INDEX_ERROR>,
          &isSubsetAtomic<TYPE_INDEX_BOOL>,
          &isSubsetAtomic<TYPE_INDEX_SPECIAL>,
          &isSubsetAtomic<TYPE_INDEX_INTMP>,
          &isSubsetAtomic<TYPE_INDEX_UCHAR>,
          &isSubsetAtomic<TYPE_INDEX_USTRING>,
          &isSubsetAtomic<TYPE_INDEX_FLOATMP>,
          &isSubsetAtomic<TYPE_INDEX_DIMENSION>,
          &subset_of_tuple,
          &subset_of_type,
          &subset_of_range,
          &subset_of_union
        };
    }

    ~TypeComparators()
    {
      delete [] m_funs;
    }

    IsSubsetOf
    getComparator(type_index t)
    {
      if (t < NUM_FUNS)
      {
        return m_funs[t];
      }
      else
      {
        return nullptr;
      }
    }

    private:
    static const int NUM_FUNS = TYPE_INDEX_UNION + 1;
    IsSubsetOf* m_funs;
  };

  static TypeComparators typeCompare;
}

//these should go in bestfit.cpp
bool
tupleApplicable(const Tuple& def, const Context& k)
{
  #if 0
  std::cerr << "tupleApplicable: def = ";
  def.print(std::cerr);
  std::cerr << std::endl;
  #endif
  //all of def has to be in c, and the values have to either be
  //equal or within the range
  for (Tuple::const_iterator iter = def.begin(); iter != def.end(); ++iter)
  {
    const Constant& val = k.lookup(iter->first);
    if (!valueRefines(val, iter->second))
    {
      return false;
    }
  }

  //if def has nothing it is applicable
  return true;
}


//does value a refine value b
bool
valueRefines(const Constant& a, const Constant& b)
{
  //this is implemented by creating a matrix of function pointers
  //which is as big as the largest type index that we need to consider


  //std::cerr << "== value refines ==" << std::endl;
  //std::cerr << a << " r " << b << std::endl;
  //if b is a range, a has to be a range and within or equal,
  //or an int and inside, otherwise they have to be equal

  IsSubsetOf f = typeCompare.getComparator(b.index());
  if (f != 0)
  {
    IsSubsetFn sub = f(a);
    return sub(a, b);
  }
  else
  {
    //there is no comparator, just check if they are equal
    return a == b;
  }
}

//does a refine b
bool
tupleRefines(const Tuple& a, const Tuple& b, bool canequal)
{
  #if 0
  std::cerr << "== tuple refines ==" << std::endl;
  a.print(std::cerr);
  std::cerr << std::endl;
  b.print(std::cerr);
  std::cerr << std::endl;
  #endif
  //for a to refine b, everything in b must be in a, and for the values that 
  //are, they have to be either equal, or their ranges must be more specific
  //but a cannot equal b
  bool equal = true;
  Tuple::const_iterator it1 = a.begin();
  Tuple::const_iterator it2 = b.begin();
  while (it1 != a.end() && it2 != b.end())
  {
    type_index d1 = it1->first;
    type_index d2 = it2->first;

    //std::cerr << "tuples have dimensions: " << d1 << " and " << d2 << 
    //  std::endl;

    //extra dimension in b
    if (d2 < d1)
    {
      //std::cerr << "no by extra dimension" << std::endl;
      return false;
    }

    //extra dimension in a
    if (d2 > d1)
    {
      ++it1;
      //a is more specific if the rest passes
      equal = false;
      //std::cerr << "not equal by dimension" << std::endl;
      continue;
    }

    if (!valueRefines(it1->second, it2->second))
    {
      //std::cerr << "no by not refines" << std::endl;
      //std::cerr << it1->first << ", " << it2->first << std::endl;
      //std::cerr << it1->second.index() << ", " << it2->second.index() 
      //  << std::endl;
      return false;
    }
    //if they both refine each other they are equal
    else if (!valueRefines(it2->second, it1->second))
    {
      //the a value is contained in the b value, so a is more
      //specific as long as the rest passes
      equal = false;
      //std::cerr << "not equal by value" << std::endl;
    }
    ++it1;
    ++it2;
  }

  if (it2 != b.end())
  {
    //b has stuff left, can't refine
    //std::cerr << "no by b iter not at end" << std::endl;
    return false;
  }

  if (it1 != a.end())
  {
    //there is stuff left in a that was never checked
    //therefore it refines
    //std::cerr << "refines" << std::endl;
    return true;
  }

  //if we get here then a is either equal to b or refines it
  //if not equal then the variable equal would have been changed somewhere
  //std::cerr << (!equal ? "yes" : "no") << std::endl;
  return !equal || canequal;
}

bool
booleanTrue(const GuardWS& g, Context& k)
{
  WS* b = g.boolean();

  if (b)
  {
    Constant v = (*b)(k);// = i.evaluate(g.boolean(), c);

    return v.index() == TYPE_INDEX_BOOL
    && get_constant<bool>(v);
  }
  else
  {
    return true;
  }
}

//the cached boolean true
bool
booleanTrue
(
  const GuardWS& g, 
  Context& kappa, 
  Context& delta, 
  std::vector<dimension_index>& demands
)
{
  WS* b = g.boolean();

  if (b)
  {
    Constant v = (*b)(kappa, delta);// = i.evaluate(g.boolean(), c);

    if (v.index() == TYPE_INDEX_DEMAND)
    {
      const auto& newd = Types::Demand::get(v);
      std::copy(newd.dims().begin(), newd.dims().end(), 
        std::back_inserter(demands));

      return false;
    }

    return v.index() == TYPE_INDEX_BOOL
    && get_constant<bool>(v);
  }
  else
  {
    return true;
  }

}

//these should go in charset.cpp
//TODO fix these
std::string
utf32_to_utf8(const std::u32string& s) {
  #ifdef ICONV_CONVERT

  std::unique_ptr<char[]> out;
  const char32_t* in = 0;
  //const size_t buffer_size = 8000;
  //if (s.size()+1 > buffer_size/sizeof(char32_t))
  //{
  //  return std::string("string too big");
  //}
  iconv_t id = iconv_open("UTF-8", "UTF-32LE");
  if (id == (iconv_t)-1)
  {
    perror("unable to open iconv: ");
  }

  //this is the number of bytes, s.size is the length in chars
  size_t inSize = s.size() * sizeof(char32_t);
  size_t outSize = s.size() * sizeof(char32_t);

  //char out[buffer_size];
  //this is the maximum size that the string will be
  //we could probably do better
  out.reset(new char[s.size() * 4 + 1]);

  //in = new char32_t[];
  //char32_t in[buffer_size];
  //memcpy(in, s.c_str(), s.size()*sizeof(char32_t));

  in = s.c_str();

  char* outp = out.get();
  //silly iconv doesn't do const
  char* inp = const_cast<char*>(reinterpret_cast<const char*>(in));

  while (inSize > 0) {
    size_t r = iconv(id, &inp, &inSize, &outp, &outSize);
    if (r == (size_t)-1)
    {
      //std::cerr << "iconv failed: " << errno << std::endl;
      perror("iconv failed 32->8: ");
      iconv_close(id);
      inSize = 0;
      return std::string();
    }
  }

  iconv_close(id);
  *outp = '\0';

  return std::string(out.get());
  #endif

  #ifdef HAND_CONVERT
  //TODO need to fix this so that all characters are output
  std::string result;
  u32string::const_iterator iter = s.begin();
  while (iter != s.end())
  {
    if (*iter <= 0x7F)
    {
      result += *iter;
    }
    else
    {
      result += ' ';
    }
    ++iter;
  }

  return result;
  #endif
}

std::u32string
utf8_to_utf32(const std::string& s)
{
  //const size_t buffer_size = 8000;
  //if (s.size()+1 > buffer_size/sizeof(char32_t))
  //{
  //  return U"string too big";
  //}
  Iconv id("UTF-32LE", "UTF-8");

  //we don't actually know how many characters the output will be
  //it is at most sizeof(char32_t) * input size
  size_t inSize = s.size();
  size_t outSize = s.size() * sizeof(char32_t);

  std::unique_ptr<char32_t[]> out(new char32_t[outSize + 1]);
  //char in[buffer_size];
  //memcpy(in, s.c_str(), s.size());

  char* outp = reinterpret_cast<char*>(out.get());
  char* inp = const_cast<char*>(s.c_str());

  while (inSize > 0) {
    size_t r = id.iconv(&inp, &inSize, &outp, &outSize);
    if (r == (size_t)-1)
    {
      perror("iconv failed, 8->32: ");
      inSize = 0;
      return std::u32string();
    }
  }

  *reinterpret_cast<char32_t*>(outp) = U'\0';
  return std::u32string(out.get());
}

std::string
u32_to_ascii(const u32string& s)
{
  std::string r;

  for(char32_t c : s)
  {
    if (c > 0x7F)
    {
      throw "character not ascii";
    }
    else
    {
      r += c;
    }
  }
  return r;
}

Constant
lookup_context(System& system, const Constant& v, const Context& k)
{
  size_t index;
  if (v.index() == TYPE_INDEX_DIMENSION)
  {
    index = get_constant<dimension_index>(v);
  }
  else
  {
    index = system.getDimensionIndex(v);
  }

  return Constant(k.lookup(index));
}

Constant
lookup_context_cached
(
  System& system, 
  const Constant& v, 
  const Context& delta
)
{
  dimension_index index;
  if (v.index() == TYPE_INDEX_DIMENSION)
  {
    index = get_constant<dimension_index>(v);
  }
  else
  {
    index = system.getDimensionIndex(v);
  }

  if (delta.has_entry(index))
  {
    return Constant(delta.lookup(index));
  }
  else
  {
    return Types::Demand::create({index});
  }
}

Tuple
makeList(const Constant& c, const Constant& tail)
{
  tuple_t list
  {
    {DIM_TYPE, Types::String::create(U"list")},
    {DIM_CONS, Types::String::create(U"cons")},
    {DIM_ARG0, c},
    {DIM_ARG1, tail}
  };

  return Tuple(list);
}

Constant
listHead(const Constant& l)
{
  return Types::Tuple::get(l).find(DIM_ARG0)->second;
}

Constant
listTail(const Constant& l)
{
  const Tuple& t = Types::Tuple::get(l);
  if (t.find(DIM_ARG1) == t.end())
  {
    throw "list has no tail";
  }
  return Types::Tuple::get(l).find(DIM_ARG1)->second;
}

std::string
read_file(std::istream& is)
{
  is.seekg(0, std::ios_base::end);
  size_t length = is.tellg();

  is.seekg(0);

  std::unique_ptr<char[]> raw(new char[length+1]);
  raw[length] = 0;

  is.read(raw.get(), length);

  return raw.get();
}

Constant
applyFunction(Context& k, const Constant& lhs, const Constant& rhs)
{
  if (lhs.index() == TYPE_INDEX_VALUE_FUNCTION)
  {
    const auto& fnval = Types::ValueFunction::get(lhs);

    return fnval.apply(k, rhs);
  }
  else
  {
    return Types::Special::create(SP_TYPEERROR);
  }
}

Constant
applyFunction
(
  Context& kappa, 
  Context& delta, 
  const Constant& lhs, 
  const Constant& rhs
)
{
  if (lhs.index() == TYPE_INDEX_VALUE_FUNCTION)
  {
    const auto& fnval = Types::ValueFunction::get(lhs);

    return fnval.apply(kappa, delta, rhs);
  }
  else
  {
    return Types::Special::create(SP_TYPEERROR);
  }
}

}
