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
  template <typename T>
  class recursive_wrapper
  {
    public:
    ~recursive_wrapper()
    {
      delete m_t;
    }

    template 
    <
      typename U,
      typename Dummy = 
        typename std::enable_if<std::is_convertible<U, T>::value, U>::type
    >
    recursive_wrapper(
      const U& u)
    : m_t(new T(u))
    {
    }

    template 
    <
      typename U,
      typename Dummy =
        typename std::enable_if<std::is_convertible<U, T>::value, U>::type
    >
    recursive_wrapper(U&& u)
    : m_t(new T(std::forward<U>(u))) { }

    recursive_wrapper(const recursive_wrapper& rhs)
    : m_t(new T(rhs.get())) { }

    recursive_wrapper(recursive_wrapper&& rhs)
    : m_t(rhs.m_t)
    {
      rhs.m_t = 0;
    }

    recursive_wrapper&
    operator=(const recursive_wrapper& rhs)
    {
      assign(rhs);
      return *this;
    }

    recursive_wrapper&
    operator=(recursive_wrapper&& rhs)
    {
      delete m_t;
      m_t = rhs.m_t;
      rhs.m_t = 0;
    }

    recursive_wrapper&
    operator=(const T& t)
    {
      assign(t);
    }

    recursive_wrapper&
    operator=(T&& t)
    {
      assign(std::move(t));
    }

    T& get() { return *m_t; }
    const T& get() const { return *m_t; }

    private:
    T* m_t;

    template <typename U>
    void
    assign(U&& u)
    {
      *m_t = std::forward<U>(u);
    }
  };

  struct true_ {};
  struct false_ {};

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

      static_assert(!std::is_convertible<Wanted, Next>::value,
        "Type is not unambiguously convertible");

      public:
      static constexpr size_t value = m_next::type::value;
      typedef typename m_next::type::type type;
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
      typedef
        typename std::conditional
        <
          std::is_convertible<Wanted, Current>::value,
          none_convertible<N, Current, Types...>,
          find_convertible<N+1, Wanted, Types...>
        >::type our_type;
      static constexpr size_t value = our_type::value;
      typedef typename our_type::type type;
    };

    #if 0
    template <size_t N, typename Wanted>
    struct find_convertible<N, Wanted>
    {
      static constexpr size_t value = N;
    };
    #endif

    //determines which out of Types... Wanted is convertible to
    template <typename Wanted, typename... Types>
    struct get_which
    {
      typedef find_convertible<0, Wanted, Types...> our_type;
      static constexpr size_t value = our_type::value;
      typedef typename our_type::type type;
    };

    template <typename T, typename Internal>
    T&
    get_value(T& t, const Internal&)
    {
      return t;
    }

    template <typename T>
    T&
    get_value(recursive_wrapper<T>& t, const false_&)
    {
      return t.get();
    }

    template <typename T>
    const T&
    get_value(const recursive_wrapper<T>& t, const false_&)
    {
      return t.get();
    }

    template
    <
      typename Internal,
      typename Visitor,
      typename VoidPtrCV
    >
    typename Visitor::result_type
    visit_impl(Visitor& visitor, int which, int current, VoidPtrCV storage,
      Internal internal)
    {
      //if your program fails here, then the visitor broke
      assert(false);
    }

    template 
    <
      typename Internal,
      typename Visitor, 
      typename VoidPtrCV,
      typename First, 
      typename... Types
    >
    typename Visitor::result_type
    visit_impl(Visitor& visitor, int which, int current, 
               VoidPtrCV storage, Internal internal = Internal())
    {
      typedef typename std::conditional
      <
        std::is_const<typename std::remove_pointer<VoidPtrCV>::type>::value,
        const First,
        First
      >::type ConstType;

      if (which == current)
      {
        return visitor(get_value(*reinterpret_cast<ConstType*>(storage), 
          internal));
      }
      return visit_impl<Internal, Visitor, VoidPtrCV, Types...>
        (visitor, which, current + 1, storage, internal);
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

    struct move_constructor
    {
      typedef void result_type;
      
      move_constructor(Variant& self)
      : m_self(self)
      {
      }

      template <typename T>
      void
      operator()(T& rhs)
      {
        m_self.construct(std::move(rhs));
      }

      private:
      Variant& m_self;
    };

    struct assigner
    {
      typedef void result_type;

      assigner(Variant& self, int rhs_which)
      : m_self(self), m_rhs_which(rhs_which)
      {
      }

      template <typename Rhs>
      void
      operator()(const Rhs& rhs)
      {
        if (m_self.which() == m_rhs_which)
        {
          //the types are the same, so just assign into the lhs
          *reinterpret_cast<Rhs*>(m_self.address()) = rhs;
        }
        else
        {
          Rhs tmp(rhs);
          m_self.destroy(); //nothrow
          m_self.construct(std::move(tmp)); //nothrow (please)
        }
      }

      private:
      Variant& m_self;
      int m_rhs_which;
    };
    
    struct move_assigner
    {
      typedef void result_type;

      move_assigner(Variant& self, int rhs_which)
      : m_self(self), m_rhs_which(rhs_which)
      {
      }

      template <typename Rhs>
      void
      operator()(Rhs& rhs)
      {
        typedef typename std::remove_const<Rhs>::type RhsNoConst;
        if (m_self.which() == m_rhs_which)
        {
          //the types are the same, so just assign into the lhs
          *reinterpret_cast<RhsNoConst*>(m_self.address()) = std::move(rhs);
        }
        else
        {
          m_self.destroy(); //nothrow
          m_self.construct(std::move(rhs)); //nothrow (please)
        }
      }

      private:
      Variant& m_self;
      int m_rhs_which;
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
      destroy();
    }

    //enable_if disables this function if we are constructing with a Variant.
    //Unfortunately, this becomes Variant(Variant&) which is a better match
    //than Variant(const Variant& rhs), so it is chosen. Therefore, we disable
    //it.
    template 
    <
      typename T, 
      typename Dummy = 
       typename std::enable_if
        <
          !std::is_same
          <
            typename std::remove_reference<Variant<First, Types...>>::type,
            typename std::remove_reference<T>::type
          >::value,
          T
        >::type
    >
    Variant(T&& t)
    {
       static_assert(
          !std::is_same<Variant<First, Types...>&, T>::value, 
          "why is Variant(T&&) instantiated with a Variant?");

      //compile error here means that T is not unambiguously convertible to
      //any of the types in (First, Types...)
      typedef typename std::remove_reference<T>::type type;
      typedef detail::get_which<type, First, Types...> which_type;

      indicate_which(which_type::value);
      construct(typename which_type::type(std::forward<T>(t)));
    }

    #if 0
    Variant(Variant& rhs)
    {
      rhs.apply_visitor_internal(constructor(*this));
      indicate_which(rhs.which());
    }
    #endif

    Variant(const Variant& rhs)
    {
      rhs.apply_visitor_internal(constructor(*this));
      indicate_which(rhs.which());
    }

    Variant(Variant&& rhs)
    {
      rhs.apply_visitor_internal(move_constructor(*this));
      indicate_which(rhs.which());
    }

    Variant& operator=(const Variant& rhs)
    {
      if (this != &rhs)
      {
        rhs.apply_visitor_internal(assigner(*this, rhs.which()));
        indicate_which(rhs.which());
      }
      return *this;
    }

    Variant& operator=(Variant&& rhs)
    {
      if (this != &rhs)
      {
        rhs.apply_visitor_internal(move_assigner(*this, rhs.which()));
        indicate_which(rhs.which());
      }
      return *this;
    }

    int which() const {return m_which;}

    template <typename Visitor, typename Internal = false_>
    typename Visitor::result_type
    apply_visitor(Visitor visitor)
    {
      return detail::visit_impl<Internal, Visitor, void*, First, Types...>(
        visitor, m_which, 0, m_storage);
    }

    template <typename Visitor, typename Internal = false_>
    typename Visitor::result_type
    apply_visitor(Visitor visitor) const
    {
      return detail::visit_impl
        <
          Internal, 
          Visitor, 
          const void*, 
          First, 
          Types...
        >(
        visitor, m_which, 0, m_storage);
    }

    private:

    //TODO implement with alignas when it is implemented in gcc
    //alignas(max<Alignof, First, Types...>::value) char[m_size];
    union
    {
      char m_storage[m_size]; //max of size + alignof for each of Types...
      //the type with the max alignment
      typename max<Alignof, First, Types...>::type m_align; 
    };

    int m_which;

    static std::function<void(void*)> m_handlers[1 + sizeof...(Types)];

    void indicate_which(int which) {m_which = which;}

    void* address() {return m_storage;}
    const void* address() const {return m_storage;}

    template <typename Visitor>
    typename Visitor::result_type
    apply_visitor_internal(Visitor visitor)
    {
      return apply_visitor<Visitor, true_>(visitor);
    }

    template <typename Visitor>
    typename Visitor::result_type
    apply_visitor_internal(Visitor visitor) const
    {
      return apply_visitor<Visitor, true_>(visitor);
    }

    void
    destroy()
    {
      apply_visitor_internal(destroyer());
    }

    template <typename T>
    void
    construct(T&& t)
    {
      typedef typename std::remove_reference<T>::type type;
      new(m_storage) type(std::forward<T>(t));
    }
  };

  struct bad_get : public std::exception
  {
    virtual const char* what() const throw()
    {
      return "TransLucid::bad_get";
    }
  };

  template <typename T>
  struct get_visitor
  {
    typedef T* result_type;

    result_type
    operator()(T& val) const
    {
      //typedef typename T::hello h;
      return &val;
    }

    template <typename U>
    result_type
    operator()(const U& u) const
    {
      return nullptr;
    }
  };

  template <typename T, typename First, typename... Types>
  T*
  get(Variant<First, Types...>* var)
  {
    return var->apply_visitor(get_visitor<T>());
  }

  template <typename T, typename First, typename... Types>
  const T*
  get(const Variant<First, Types...>* var)
  {
    return var->apply_visitor(get_visitor<const T>());
  }

  template <typename T, typename First, typename... Types>
  T&
  get (Variant<First, Types...>& var)
  {
    T* t = var.apply_visitor(get_visitor<T>());
    if (t == nullptr){throw bad_get();}

    return *t;
  }

  template <typename T, typename First, typename... Types>
  const T&
  get (const Variant<First, Types...>& var)
  {
    const T* t = var.apply_visitor(get_visitor<const T>());
    if (t == nullptr) {throw bad_get();}

    return *t;
  }
}
