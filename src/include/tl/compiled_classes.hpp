/* Definitions for expressions being compiled to c++.
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

#ifndef COMPILED_CLASSES_HPP_INCLUDED
#define COMPILED_CLASSES_HPP_INCLUDED

#include <string>

#if 0
#include <tl/types.hpp>

namespace TransLucid
{
  typedef <typename T, K N>
  class Constant
  {
    public:

    typedef T RawType;

    RawType
    operator()(Tuple& k)
    {
      return N;
    }

    private:
  };

  typedef <typename T, int32_t N>
  class Constant
  {
    public:

    T
    operator()(Tuple& k)
    {
      return N;
    }

    private:
  };
#endif

extern std::string x;

  template <std::string* N>
  class F
  {
  };

  int
  main()
  {
    //std::string x;
    F<&x> y;
  }

  //int32<3> + int32<4>;;
  //
  //Constant<int32_t, 3>()() + Constant<int32_t, 4>()()

  //Constant<int32_t>(4)() + Constant<int32>(3)();

  //Constant<Glib::ustring,"h">

#if 0
}
#endif

#endif // COMPILED_CLASSES_HPP_INCLUDED
