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

#include <tl/utility.hpp>
#include <tl/equation.hpp>
#include <tl/types.hpp>
#include <tl/types/intmp.hpp>
#include <tl/types/special.hpp>
#include <tl/types/range.hpp>
#include <tl/system.hpp>
#include <tl/types/type.hpp>

#ifdef ICONV_CONVERT
#include <iconv.h>
#endif

namespace TransLucid
{

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
  //std::cerr << "== value refines ==" << std::endl;
  //std::cerr << a << " r " << b << std::endl;
  //if b is a range, a has to be a range and within or equal,
  //or an int and inside, otherwise they have to be equal

  //three cases, range, type, anything else

  if (b.index() == TYPE_INDEX_RANGE)
  {
    if (a.index() == TYPE_INDEX_RANGE)
    {
      if (!Types::Range::get(b).within(Types::Range::get(a)))
      {
        //std::cerr << "no" << std::endl;
        return false;
      }
    }
    else if (a.index() == TYPE_INDEX_INTMP)
    {
      if (!Types::Range::get(b).within(Types::Intmp::get(a)))
      {
        //std::cerr << "no" << std::endl;
        return false;
      }
    }
    else
    {
        //std::cerr << "no" << std::endl;
      return false;
    }
        //std::cerr << "yes" << std::endl;
    return true;
  }
  else if (b.index() == TYPE_INDEX_TYPE)
  {
    //std::cerr << "type type index == " << b.value<Type>().index()
    //<< std::endl;
    if (a.index() == get_constant<type_index>(b))
    {
        //std::cerr << "yes" << std::endl;
      return true;
    }
    else
    {
        //std::cerr << "no" << std::endl;
      return false;
    }
  }
  else
  {
    //std::cerr << (a==b ? "yes" : "no") << std::endl;
    return a == b;
  }

  //for now a and b just have to be equal
    //std::cerr << (a==b ? "yes" : "no") << std::endl;
  return a == b;
}

//does a refine b
bool
tupleRefines(const Tuple& a, const Tuple& b)
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

    //extra dimension in b
    if (d2 < d1)
    {
      //std::cerr << "no by extra dimension" << std::endl;
      return false;
    }

    //extra dimension in a
    if (d1 > d2)
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

  //if we get here then a is either equal to be or refines it
  //if not equal then the variable equal would have been changed somewhere
  //std::cerr << (!equal ? "yes" : "no") << std::endl;
  return !equal;
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
  iconv_t id = iconv_open("UTF8", "UTF32LE");
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
  out.reset(new char[s.size() * 4]);

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
  Iconv id("UTF32LE", "UTF8");

  //we don't actually know how many characters the output will be
  //it is at most sizeof(char32_t) * input size
  size_t inSize = s.size();
  size_t outSize = s.size() * sizeof(char32_t);

  std::unique_ptr<char32_t[]> out(new char32_t[outSize]);
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

#if 0
  Tuple::const_iterator iter = k.find(index);
  if (iter != k.end())
  {
    return TaggedConstant(iter->second, k);
  }
  else
  {
    //find the all dimension
    Tuple::const_iterator all = k.find(DIM_ALL);
    if (all == k.end())
    {
      return TaggedConstant(Types::Special::create(SP_DIMENSION), k);
    }
    else
    {
      return TaggedConstant(all->second, k);
    }
  }
#endif
}

}
