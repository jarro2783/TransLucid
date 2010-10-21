/* OpHD class.
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

#include <tl/hyperdaton.hpp>

namespace TransLucid
{
  namespace OpHDImp
  {
    template <template <typename...> class T, typename ...Args1>
    struct concatenate
    {
      template <typename ...Args2>
      struct with
      {
        typedef T<Args1..., Args2...> type;
      };
    };

    template <type_index ...Args>
    struct generate_args_tuple_type;

    template <>
    struct generate_args_tuple_type<>
    {
      typedef size_t type;
    };

    //creates a tuple with length of Args integers as parameters
    template <type_index First, type_index ...Args>
    struct generate_args_tuple_type<First, Args...>
    {
      typedef typename concatenate<std::tuple, size_t>::
        with<generate_args_tuple_type<Args...>>::type type;
    };
  }

  //T is the actual functor
  //ArgType is a template which ::type will return the type of an argument
  //by its index, Args are the indices
  //this is the index-known-at-compile-time version
  template <typename T, 
    template <typename> class ArgType, type_index ...Args>
  class OpHD : public HD
  {
    //generates all of the args, then extracts them and checks that they
    //are the right type and passes them all to the functor
    public:
    OpHD(SystemHD& i)
    {
    }

    TaggedConstant operator()(const Tuple& c)
    {
    }

    private:
    typedef typename OpHDImp::generate_args_tuple_type<Args...>::type DimsType;
    DimsType m_args;

    T m_t;
  };
}
