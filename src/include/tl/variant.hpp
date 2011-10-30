/* A tagged union variant class.
   Copyright (C) 2011 Jarryd Beck

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

/**
 * @file system.hpp
 * A tagged union. This effectively has the same functionality as
 * boost::variant, but replaces it with C++11 features.
 */

#include <cassert>
#include <functional>
#include <new>
#include <type_traits>
#include <utility>

//#include <iostream>

#include <tl/mpl.hpp>

namespace TransLucid
{
  namespace detail
  {
    //none of what is left in Types should be convertible to Wanted
    template <size_t N, typename Wanted, typename... Types>
    struct none_convertible;

    template <size_t N, typename Wanted, typename Next, typename... Types>
    struct none_convertible<N, Wanted, Next, Types...>
    {
      private:
      //if you get an error here it means that a type that your variant was
      //constructed with is not unambiguously convertible to one of the 
      //types in the variant
      typedef 
      std::enable_if
      <
        !std::is_convertible<Wanted, Next>::value,
        none_convertible<N, Wanted, Types...>
      > m_next;

      public:
      static constexpr size_t value = m_next::type::value;
    };

    template <size_t N, typename Wanted>
    struct none_convertible<N, Wanted>
    {
      typedef Wanted type;
      static constexpr size_t value = N;
    };

    template <size_t N, typename Wanted, typename... Types>
    struct find_convertible;

    template <size_t N, typename Wanted, typename Current, typename... Types>
    struct find_convertible<N, Wanted, Current, Types...>
    {
      static constexpr size_t value =
        std::conditional
        <
          std::is_convertible<Wanted, Current>::value,
          none_convertible<N, Wanted, Types...>,
          find_convertible<N+1, Wanted, Types...>
        >::type::value;
    };

    #if 0
    template <size_t N, typename Wanted>
    struct find_convertible<N, Wanted>
    {
      static_assert(false, "Type not convertible");
    };
    #endif

    //determines which out of Types... Wanted is convertible to
    template <typename Wanted, typename... Types>
    struct get_which
    {
      static constexpr size_t value = 
        find_convertible<0, Wanted, Types...>::value;
    };

    template
    <
      typename Visitor,
      typename VoidPtrCV
    >
    typename Visitor::result_type
    visit_impl(Visitor& visitor, int which, int current, VoidPtrCV storage)
    {
      //if your program fails here, then the visitor broke
      assert(false);
    }

    template 
    <
      typename Visitor, 
      typename VoidPtrCV,
      typename First, 
      typename... Types
    >
    typename Visitor::result_type
    visit_impl(Visitor& visitor, int which, int current, 
               VoidPtrCV storage)
    {
      typedef typename std::conditional
      <
        std::is_const<VoidPtrCV>::value,
        First,
        const First
      >::type ConstType;
      //TODO constness here
      if (which == current)
      {
        return visitor(*reinterpret_cast<ConstType*>(storage));
      }
      return visit_impl<Visitor, VoidPtrCV, Types...>
        (visitor, which, current + 1, storage);
    }
  }

  template <typename First, typename... Types>
  class Variant
  {
    private:

    template <typename T>
    struct Sizeof
    {
      static constexpr size_t value = sizeof(T);
    };

    template <typename T>
    struct Alignof
    {
      static constexpr size_t value = alignof(T);
    };

    //size = max of size of each thing
    static constexpr size_t m_size = 
      max
      <
        Sizeof,
        First,
        Types...
      >::value;

    struct constructor
    {
      typedef void result_type;

      constructor(Variant& self)
      : m_self(self)
      {
      }

      template <typename T>
      void
      operator()(const T& rhs)
      {
        m_self.construct(rhs);
      }

      private:
      Variant& m_self;
    };

    struct destroyer
    {
      typedef void result_type;

      template <typename T>
      void
      operator()(T& t)
      {
        t.~T();
      }
    };

    public:

    Variant()
    {
      //try to construct First
      //if this fails then First is not default constructible
      construct(First());
      indicate_which(0);
    }

    ~Variant()
    {
      apply_visitor(destroyer());
    }

    template <typename T>
    Variant(T t)
    {
      //compile error here means that T is not unambiguously convertible to
      //any of the types in (First, Types...)
      indicate_which(detail::get_which<T, First, Types...>::value);
      construct(std::forward<T>(t));
      //std::cerr << "using which = " << m_which << std::endl;
    }

    Variant(const Variant& rhs)
    {
      rhs.apply_visitor(constructor(*this));
    }

    Variant(Variant&& rhs);

    Variant& operator=(const Variant& rhs);

    Variant& operator=(Variant&& rhs);

    int which() {return m_which;}

    template <typename Visitor>
    typename Visitor::result_type
    apply_visitor(Visitor visitor)
    {
      return detail::visit_impl<Visitor, void*, First, Types...>(
        visitor, m_which, 0, m_storage);
    }

    template <typename Visitor>
    typename Visitor::result_type
    apply_visitor(Visitor visitor) const
    {
      return detail::visit_impl<Visitor, const void*, First, Types...>(
        visitor, m_which, 0, m_storage);
    }

    private:

    //TODO implement with alignas when it is implemented in gcc
    union
    {
      char m_storage[m_size]; //max of size + alignof for each of Types...
      //the type with the max alignment
      typename max<Alignof, First, Types...>::type m_align; 
    };

    int m_which;

    static std::function<void(void*)> m_handlers[1 + sizeof...(Types)];

    void indicate_which(int which) {m_which = which;}

    template <typename T>
    void
    construct(T&& t)
    {
      new(m_storage) T(std::forward<T>(t));
    }
  };
}
