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

#include <tl/utility.hpp>
#include <tl/equation.hpp>
#include <tl/types.hpp>
#include <iconv.h>

namespace TransLucid
{

const u32string
SplitID::m_split_match = U".";

bool
tupleApplicable(const Tuple& def, const Tuple& c)
{
  //all of def has to be in c, and the values have to either be
  //equal or within the range
  for (Tuple::const_iterator iter = def.begin(); iter != def.end(); ++iter)
  {
    Tuple::const_iterator citer = c.find(iter->first);
    if (citer == c.end())
    {
      return false;
    }
    else
    {
      if (!valueRefines(citer->second, iter->second))
      {
        return false;
      }
    }
  }

  //if def has nothing it is applicable
  return true;
}


//does value a refine value b
bool
valueRefines(const Constant& a, const Constant& b)
{
  //if b is a range, a has to be a range and within or equal,
  //or an int and inside, otherwise they have to be equal

  //three cases, range, type, anything else

  if (b.index() == TYPE_INDEX_RANGE)
  {
    if (a.index() == TYPE_INDEX_RANGE)
    {
      if (!b.value<Range>().within(a.value<Range>()))
      {
        return false;
      }
    }
    else if (a.index() == TYPE_INDEX_INTMP)
    {
      if (!b.value<Range>().within(a.value<Intmp>()))
      {
        return false;
      }
    }
    else
    {
      return false;
    }
    return true;
  }
  else if (b.index() == TYPE_INDEX_TYPE)
  {
    //std::cerr << "type type index == " << b.value<Type>().index()
    //<< std::endl;
    if (a.index() == b.value<Type>().index())
    {
      return true;
    }
    else
    {
      return false;
    }
  }
  else
  {
    return a == b;
  }

  //for now a and b just have to be equal
  return a == b;
}

//does a refine b
bool
tupleRefines(const Tuple& a, const Tuple& b)
{
  //for a to refine b, everything in b must be in a, and for the values that 
  //are, they have to be either equal, or their ranges must be more specific
  Tuple::const_iterator it1 = a.begin();
  Tuple::const_iterator it2 = b.begin();
  while (it1 != a.end() && it2 != b.end())
  {
    type_index d1 = it1->first;
    type_index d2 = it2->first;

    //extra dimension in b
    if (d2 < d1)
    {
      return false;
    }

    //extra dimension in a
    if (d1 > d2)
    {
      ++it1;
      continue;
    }

    if (!valueRefines(it1->second, it2->second))
    {
      return false;
    }
    ++it1;
    ++it2;
  }

  if (it2 != b.end())
  {
    return false;
  }
  return true;
}

bool
booleanTrue(const GuardHD& g, const Tuple& k)
{
  HD* b = g.boolean();

  if (b)
  {
    TaggedConstant v = (*b)(k);// = i.evaluate(g.boolean(), c);

    return v.first.index() == TYPE_INDEX_BOOL
    && v.first.value<Boolean>();
  }
  else
  {
    return true;
  }
}

std::string
utf32_to_utf8(const std::u32string& s) {
  const size_t buffer_size = 8000;
  if (s.size()+1 > buffer_size/sizeof(char32_t))
  {
    return std::string("string too big");
  }
  iconv_t id = iconv_open("UTF8", "UTF32LE");
  if (id == (iconv_t)-1)
  {
    perror("unable to open iconv: ");
  }

  size_t inSize = (s.size())* sizeof(char32_t);
  size_t outSize = buffer_size * sizeof(char32_t);
  char out[buffer_size];
  char32_t in[buffer_size];
  memcpy(in, s.c_str(), s.size()*sizeof(char32_t));

  char* outp = out;
  char* inp = reinterpret_cast<char*>(in);

  while (inSize > 0) {
    size_t r = iconv(id, &inp, &inSize, &outp, &outSize);
    if (r == (size_t)-1)
    {
      //std::cerr << "iconv failed: " << errno << std::endl;
      perror("iconv failed: ");
      inSize = 0;
      return std::string();
    }
  }

  iconv_close(id);
  *outp = '\0';
  return std::string(out);
}

std::u32string
utf8_to_utf32(const std::string& s)
{
  const size_t buffer_size = 8000;
  if (s.size()+1 > buffer_size/sizeof(char32_t))
  {
    return U"string too big";
  }
  Iconv id("UTF32LE", "UTF8");

  //we don't actually know how many characters the output will be
  //it is at most sizeof(char32_t) * input size
  size_t inSize = s.size();
  size_t outSize = buffer_size * sizeof(char32_t);
  char32_t out[buffer_size];
  char in[buffer_size];
  memcpy(in, s.c_str(), s.size());

  char* outp = reinterpret_cast<char*>(out);
  char* inp = in;

  while (inSize > 0) {
    size_t r = id.iconv(&inp, &inSize, &outp, &outSize);
    if (r == (size_t)-1)
    {
      //std::cerr << "iconv failed: " << errno << std::endl;
      perror("iconv failed: ");
      inSize = 0;
      return std::u32string();
    }
  }

  *outp = '\0';
  return std::u32string(out);
}

std::string
u32_to_ascii(const u32string& s)
{
  std::string r;

  BOOST_FOREACH(char32_t c, s)
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

}
