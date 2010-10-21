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
    template 
    <
      template <typename...> class List,
      template<type_index> class Mod,
      type_index ...Args
    >
    struct modify_type_index_list
    {
      typedef List<typename Mod<Args>::type...> type;
    };

    template <type_index T>
    struct make_size_t
    {
      typedef size_t type;
    };

    //creates a tuple with length of Args integers as parameters
    template <type_index ...Args>
    struct generate_args_tuple_type
    {
      typedef typename modify_type_index_list
        <std::tuple, make_size_t, Args...>::type type;
    };

    struct lookup_index
    {
      lookup_index(SystemHD& i)
      {
      }

      size_t operator()(type_index index)
      {
      }
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
    private:
    typedef typename OpHDImp::generate_args_tuple_type<Args...>::type DimsType;
    DimsType m_args;

    //generates all of the args, then extracts them and checks that they
    //are the right type and passes them all to the functor
    public:
    OpHD(SystemHD& i)
    {
      m_args = DimsType(OpHDImp::lookup_index(i)(Args)...);
    }

    TaggedConstant operator()(const Tuple& c)
    {
    }

    private:
    T m_t;
  };
}
