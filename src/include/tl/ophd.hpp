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

    //TODO this needs to lookup the argN indices and build the List
    //template <template <typename...> class List>
    struct lookup_index
    {
      lookup_index(SystemHD& i)
      : m_i(i)
      {
      }

      size_t operator()(type_index index) const
      {
        //return get_type_index(&m_i, 
      }

      SystemHD& m_i;
    };

    //TODO finish this
    template
    <
      template <int> class ArgType,
      type_index type
    >
    struct make_constant
    {
      typedef typename ArgType<type>::type ret;

      ret 
      operator()(const Tuple& c, size_t index)
      {
        Tuple::const_iterator iter = c.find(index);
        if (iter != c.end())
        {
          if (iter->second.index() == type)
          {
            return iter->second.value<ret>();
          }
        }
      }
    };

    template
    <
      int N, 
      template <int> class ArgType,
      type_index ...Args
    >
    struct build_arguments_imp;

    template
    <
      int N,
      template <int> class ArgType
    >
    struct build_arguments_imp<N, ArgType>
    {
      template
      <
        typename F,
        typename List,
        typename ...Constants
      >
      TaggedConstant
      operator()
      (
        const F& f, const List& args, const Tuple& c, Constants... constants
      )
      {
        return f(constants...);
      }
    };

    template
    <
      int N, 
      template <int> class ArgType,
      type_index First,
      type_index ...Args
    >
    struct build_arguments_imp<N, ArgType, First, Args...>
    {
      template 
      <
        typename F, 
        typename List,
        typename ...Constants
      >
      TaggedConstant 
      operator()
      (
        const F& f, 
        const List& args, 
        const Tuple& c,
        Constants... constants
      )
      {
        return build_arguments_imp<N+1, ArgType, Args...>()
          (f, args, c, constants..., 
            make_constant<ArgType, First>()(c, std::get<N>(args)));
      }
    };
 
    template
    <
      template <int> class ArgType,
      type_index ...Args
    >
    struct build_arguments
    {
      template 
      <
        typename F, 
        typename List
      >
      TaggedConstant 
      operator()
      (
        const F& f, 
        const List& args, 
        const Tuple& c
      )
      {
        return build_arguments_imp<0, ArgType, Args...>()(f, args, c);
      }
    };
  }

  //T is the actual functor
  //ArgType is a template which ::type will return the type of an argument
  //by its index, Args are the indices
  //this is the index-known-at-compile-time version
  template <typename T, 
    template <int> class ArgType, type_index ...Args>
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
      return OpHDImp::build_arguments<ArgType, Args...>()(m_t, m_args, c);
    }

    private:
    T m_t;
  };
}
