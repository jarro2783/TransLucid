/* Meta-programming utility.
   Copyright (C) 2011, 2012 Jarryd Beck

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

#ifndef TL_MPL_HPP_INCLUDED
#define TL_MPL_HPP_INCLUDED

#include <type_traits>
#include <cstdlib>
#include <utility>

namespace TransLucid
{
  namespace detail
  {
    template 
    <
      template <typename> class Size,
      typename SoFar, 
      typename... Args
     >
    struct max_helper;

    template 
    <
      template <typename> class Size,
      typename SoFar
    >
    struct max_helper<Size, SoFar>
    {
      static constexpr decltype(Size<SoFar>::value) value = 
        Size<SoFar>::value;
      typedef SoFar type;
    };

    template 
    <
      template <typename> class Size,
      typename SoFar, 
      typename Next, 
      typename... Args
    >
    struct max_helper<Size, SoFar, Next, Args...>
    {
      private:
      typedef typename std::conditional
      <
        (Size<Next>::value > Size<SoFar>::value),
        max_helper<Size, Next, Args...>,
        max_helper<Size, SoFar, Args...>
      >::type m_next;

      public:
      static constexpr decltype(Size<SoFar>::value) value = 
        m_next::value;

      typedef typename m_next::type type;
    };
  }

  template <template <typename> class Size, typename... Args>
  struct max;

  template 
  <
    template <typename> class Size, 
    typename First, 
    typename... Args
  >
  struct max<Size, First, Args...>
  {
    private:
    typedef decltype(Size<First>::value) m_size_type;
    typedef detail::max_helper
    <
      Size, 
      First, 
      Args...
    > m_helper;

    public:
    static constexpr m_size_type value = m_helper::value;
    typedef typename m_helper::type type;
  };

  template <typename F>
  struct count_args;

  template <typename Ret, typename... Args>
  struct count_args<Ret(Args...)>
  {
    static constexpr size_t value = sizeof...(Args);
  };

  template <typename C, typename Ret, typename... Args>
  struct count_args<Ret (C::*)(Args...)>
  {
    static constexpr size_t value = sizeof...(Args);
  };

  template 
  <
    typename F, 
    typename Dummy = 
      typename std::enable_if<!std::is_function<F>::value, F>::type
  >
  struct count_args<F, Dummy>
  {
    static constexpr size_t value =
      count_args<decltype(&F::operator())>::value;
  };
}

#endif
