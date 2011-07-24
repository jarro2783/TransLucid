/* The function registry.
   Copyright (C) 2009-2011 Jarryd Beck and John Plaice

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

#ifndef TL_FUNCTION_REGISTRY_HPP_INCLUDED
#define TL_FUNCTION_REGISTRY_HPP_INCLUDED

#include <tl/types.hpp>

#include <functional>
#include <unordered_map>

namespace TransLucid
{
  template <typename T>
  struct identity
  {
    typedef T type;
  };

  template <size_t N, typename... Args>
  struct make_function_type
  {
    typedef typename make_function_type<N-1, Constant, Args...>::type type;
  };

  template <typename... Args>
  struct make_function_type<0, Args...>
  {
    //typedef typename identity<Constant(*)(Args...)>::type type;
    typedef typename std::function<Constant(Args...)> type;
  };

  template <size_t N, typename... Types>
  struct make_tuple_type
  {
    typedef typename 
    make_tuple_type
    <
      N-1, 
      typename make_function_type<N>::type, 
      Types...
    >::type
      type;
  };

  template <typename... Types>
  struct make_tuple_type<-1, Types...>
  {
    typedef typename std::tuple<std::unordered_map<u32string,Types>...> type;
  };

  template <typename T>
  void die(T t)
  {
    t.foo();
  }

  template <size_t N>
  class FunctionRegistry
  {
    public:
    FunctionRegistry() = default;

    //an error here probably means that the function isn't of type
    //Constant(Constant...)
    template <typename... Args>
    void
    registerFunction
    (
      const u32string& name, 
      std::function<Constant(Args... args)> f
    )
    {
      //die(m_functions);
      static_assert(sizeof...(Args) <= N, "function takes too many arguments");
      std::get<sizeof...(Args)>(m_functions).insert
      (
        std::make_pair(name, f)
      );
    }

    private:

    typedef typename make_tuple_type<N>::type storage_type;
    storage_type m_functions;

    public:

    //so that this compiles with gcc 4.6, use this type definition,
    //otherwise the commented out one should work
    template <size_t U>
    typename make_function_type<U>::type
    //auto
    lookupFunction(const u32string& name)
    //  -> decltype(std::get<U>(m_functions).find(name)->second)
    {
      static_assert(U <= N, "too many arguments");
      auto iter = std::get<U>(m_functions).find(name);
      if (iter != std::get<U>(m_functions).end())
      {
        return iter->second;
      }
      else
      {
        //return nullptr;
        return typename make_function_type<U>::type();
      }
    }
  };
}

#endif
