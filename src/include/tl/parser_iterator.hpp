/* Parser forward declarations.
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

#ifndef PARSER_ITERATOR_HPP_INCLUDED
#define PARSER_ITERATOR_HPP_INCLUDED

#include <iterator>
#include <typeinfo>
#include <iostream>

#include <tl/charset.hpp>

namespace TransLucid
{
  namespace Parser
  {
    struct iterator_traits :
      public std::iterator
      <
      //TODO work out what type of iterator this is
      //maybe I should make it a forward iterator and implement my
      //own buffering when reading straight from cin.
        std::input_iterator_tag,
        //std::forward_iterator_tag,
        unsigned int,
        int64_t
      >
    {
    };

    class Iterator : public iterator_traits
    {
      public:
      virtual ~Iterator() = 0;
      virtual Iterator* clone() const = 0;
      virtual bool operator==(const Iterator& rhs) const = 0;
      virtual Iterator& operator++() = 0;
      virtual reference operator*() const = 0;
      virtual pointer operator->() const = 0;
    };

    inline 
    Iterator::~Iterator()
    {
    }

    class U32Iterator : public iterator_traits
    {
      public:

      U32Iterator()
      : m_iter(0)
      , m_end(0)
      {
      }

      U32Iterator(const Iterator& i, const Iterator& end)
      : m_iter(i.clone())
      , m_end(end.clone())
      {
      }

      U32Iterator(const U32Iterator& other)
      : m_iter(other.m_iter != 0 ? other.m_iter->clone() : 0)
      , m_end(other.m_end != 0 ? other.m_end->clone() : 0)
      {
      }

      U32Iterator& operator=(const U32Iterator& rhs)
      {
        if (this != &rhs)
        {
          Iterator* iter_copy = 0;
          Iterator* end_copy = 0;

          try
          {
            iter_copy = rhs.m_iter != 0 ? rhs.m_iter->clone() : 0;
            end_copy = rhs.m_end != 0 ? rhs.m_end->clone() : 0;

            delete m_iter;
            delete m_end;

            m_iter = iter_copy;
            m_end = end_copy;
          }
          catch (...)
          {
            delete iter_copy;
            delete end_copy;
            throw;
          }
        }
        return *this;
      }
      
      bool operator==(const U32Iterator& rhs) const
      {
        bool lhs_end = false;
        bool rhs_end = false;

        if (m_iter == 0 || *m_iter == *m_end)
        {
          lhs_end = true;
        }

        if (rhs.m_iter == 0 || *rhs.m_iter == *rhs.m_end)
        {
          rhs_end = true;
        }

        return lhs_end == rhs_end;
      }

      bool operator!=(const U32Iterator& rhs) const
      {
        return !this->operator==(rhs);
      }

      U32Iterator& operator++()
      {
        ++*m_iter;
        return *this;
      }

      const U32Iterator operator++(int)
      {
        U32Iterator old(*this);
        ++*this;
        return old;
      }

      reference operator*() const
      {
        //u32string s;
        //s += **m_iter;
        //std::cerr << utf32_to_utf8(s) << std::endl;
        return **m_iter;
      }

      pointer operator->() const
      {
        return m_iter->operator->();
      }

      private:
      Iterator* m_iter;
      Iterator* m_end;
    };

    /**
     * Iterates through a UTF-8 stream.
     */
    template <typename T>
    class UTF8Iterator : public Iterator
    {
      public:
      UTF8Iterator(const T& iter)
      : m_iter(iter), m_haveReadCurrent(false)
      {
      }

      UTF8Iterator(const UTF8Iterator& other)
      : m_iter(other.m_iter)
      , m_value(other.m_value)
      , m_haveReadCurrent(other.m_haveReadCurrent)
      {
      }

      UTF8Iterator* clone() const
      {
        return new UTF8Iterator(*this);
      }

      bool operator==(const Iterator& rhs) const
      {
        try
        {
          const UTF8Iterator& crhs = dynamic_cast<const UTF8Iterator&>(rhs);
          return m_iter == crhs.m_iter;
        }
        catch (std::bad_cast&)
        {
          return false;
        }
      }

      Iterator& operator++() 
      {
        if (!m_haveReadCurrent)
        {
          readNext();
        }

        m_haveReadCurrent = false;

        return *this;
      }

      reference operator*() const
      {
        if (!m_haveReadCurrent)
        {
          readNext();
        }
        //std::cerr << utf32_to_utf8(u32string({m_value})) << std::endl;;
        return m_value;
      }

      pointer operator->() const
      {
        if (!m_haveReadCurrent)
        {
          readNext();
        }
        return &m_value;
      }

      private:

      void readNext() const
      {
        //std::cerr << "readNext() ";
        m_value = 0;
        char c = *m_iter;
        int toRead = 0;
        int nextShift = 0;

        //std::cerr << c << " ";

        if ((c & 0x80) == 0)
        {
          //ascii
          m_value = c;
        }
        else if ((c & 0xE0) == 0xC0)
        {
          //two characters
          toRead = 1;
          m_value = 0x7C0 & (c << 6);
          nextShift = 0;
        }
        else if ((c & 0xF0) == 0xE0)
        {
          //three characters
          toRead = 2;
          nextShift = 6;
        }
        else if ((c & 0xF8) == 0xF0)
        {
          //four characters
          toRead = 3;
          nextShift = 12;
        }
        else
        {
          //invalid character
        }

        ++m_iter;

        for (int i = 0; i != toRead; ++i)
        {
          c = *m_iter;
          //std::cerr << c << " ";
          if ((c & 0xC0) != 0x80)
          {
            //invalid character
          }
          m_value |= (0x3F << nextShift);
          nextShift -= 6;
          ++m_iter;
        }

        //std::cerr << std::endl;

        //std::cerr << "readNext() " << m_value << std::endl;

        m_haveReadCurrent = true;
      }

      mutable T m_iter;
      mutable value_type m_value;
      mutable bool m_haveReadCurrent;
    };

    template <typename T>
    UTF8Iterator<T> makeUTF8Iterator(const T& iter)
    {
      return UTF8Iterator<T>(iter);
    }

    template <typename T>
    struct remove_reference
    {
      typedef T type;
    };

    template <typename T>
    struct remove_reference<T&>
    {
      typedef T type;
    };

    template <typename T>
    struct remove_reference<T*>
    {
      typedef T type;
    };

    template <typename T>
    struct create_reference
    {
    };

    template <typename T>
    struct create_reference<T&>
    {
      T& operator()(T& t)
      {
        return t;
      }
    };

    template <typename T>
    struct create_reference<T*>
    {
      T* operator()(T& t)
      {
        return &t;
      }
    };

    template <typename T>
    struct dereference
    {
      T operator()(T t)
      {
        return t;
      }
    };

    template <typename T>
    struct dereference<T*>
    {
      T& operator()(T* t)
      {
        return *t;
      }
    };

    template <typename HasType, typename NeedsType>
    struct Buffer
    {
      NeedsType operator()(HasType v)
      {
        m_buf = dereference<HasType>()(v);
        return create_reference<NeedsType>()(m_buf);
      }

      typename remove_reference<NeedsType>::type m_buf;
    };

    template <typename HasType>
    struct Buffer<HasType, HasType>
    {
      HasType operator()(HasType v)
      {
        return v;
      }
    };

    template <typename HasType, typename NeedsRetType>
    struct IteratorDereference
    {
      NeedsRetType operator()(HasType v) const
      {
        return m_buf(v);
      }

      mutable Buffer<HasType, NeedsRetType> m_buf;
    };

    template <typename T>
    class UTF32Iterator : public Iterator
    {
      public:
      UTF32Iterator(const T& iter)
      : m_iter(iter)
      {
      }

      UTF32Iterator(const UTF32Iterator& other)
      : m_iter(other.m_iter)
      , m_reference(other.m_reference)
      , m_pointer(other.m_pointer)
      {
      }

      Iterator* clone() const
      {
        return new UTF32Iterator(*this);
      }

      bool operator==(const Iterator& rhs) const
      {
        try
        {
          const UTF32Iterator<T>& rhs_cast = 
            dynamic_cast<const UTF32Iterator<T>&>(rhs);
          return m_iter == rhs_cast.m_iter;
        }
        catch (std::bad_cast)
        {
          //std::cerr << "bad cast in operator==" << std::endl;
          return false;
        }
      }

      Iterator& operator++()
      {
        ++m_iter;
        return *this;
      }

      reference operator*() const
      {
        return m_reference(*m_iter);
      }

      pointer operator->() const
      {
        return m_pointer(m_iter.operator->());
      }

      private:
      T m_iter;
      IteratorDereference<typename T::reference, reference> m_reference;
      IteratorDereference<typename T::pointer, pointer> m_pointer;
    };

    template <typename T>
    UTF32Iterator<T>
    makeUTF32Iterator(const T& iter)
    {
      return UTF32Iterator<T>(iter);
    }

  } //namespace Parser
} //namespace TransLucid

#endif
