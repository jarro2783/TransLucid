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
        std::input_iterator_tag,
        wchar_t
      >
    {
    };

    /**
     * A unicode iterator. The base class for the unicode iterators.
     */
    class Iterator : public iterator_traits
    {
      public:
      virtual ~Iterator() = 0;
      
      /**
       * Clone an iterator.
       * @return The cloned iterator.
       */
      virtual Iterator* clone() const = 0;

      /**
       * Equality. Checks if two iterators are equal.
       */
      virtual bool operator==(const Iterator& rhs) const = 0;

      /**
       * Increment. Increments the iterator and returns a reference to itself.
       * @return *this.
       */
      virtual Iterator& operator++() = 0;

      /**
       * Dereference. Dereference the iterator and return a reference to
       * the value.
       * @return A reference to the value which the iterator points to.
       */
      virtual reference operator*() const = 0;

      /**
       * Dereference. Dereference the iterator and return a pointer to the
       * value.
       * @return A pointer to the value which the iterator points to.
       */
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

      /**
       * Construct a U32Iterator.
       * We need to know where the end of the stream is to decide when we
       * have reached the end.
       */
      U32Iterator(const Iterator& i, const Iterator& end)
      : m_iter(i.clone())
      , m_end(end.clone())
      {
      }

      /**
       * Copy construct a U32Iterator.
       * @param other The U32Iterator to make a copy of.
       */
      U32Iterator(const U32Iterator& other)
      : m_iter(other.m_iter != 0 ? other.m_iter->clone() : 0)
      , m_end(other.m_end != 0 ? other.m_end->clone() : 0)
      {
      }

      /**
       * Assign two iterators.
       * Clones the underlying iterators of the rhs and assigns them to
       * *this.
       * @param rhs The iterator to assign to.
       * @return *this.
       */
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
      
      /**
       * Equality test. Tests if two iterators are equal.
       * Two iterators are equal if they are both end iterators
       * or if they are the same iterator.
       * An iterator is an end iterator if its iterator is zero or if
       * its iterator equals its end iterator.
       * @param rhs The iterator to compare to.
       * @return @b True if the iterators are equal.
       */
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

      /**
       * Equality test.
       * @param rhs The other iterator to compare to.
       * @return !(*this == rhs)
       * @see U32Iterator::operator==
       */
      bool operator!=(const U32Iterator& rhs) const
      {
        return !this->operator==(rhs);
      }

      /**
       * Pre-increment. Increments the iterator.
       * @return The value after being iterated.
       */
      U32Iterator& operator++()
      {
        ++*m_iter;
        return *this;
      }

      /**
       * Post-increment. Increments the iterator.
       * @return The value before being iterated.
       */
      const U32Iterator operator++(int)
      {
        U32Iterator old(*this);
        ++*this;
        return old;
      }

      /**
       * Dereference operator. Returns a reference.
       * @return A reference to the pointed to value.
       */
      reference operator*() const
      {
        //std::cerr << u32string(1, **m_iter) << std::endl;
        return **m_iter;
      }

      /**
       * Dereference operator. Returns a pointer.
       * @return A pointer to the pointed to value.
       */
      pointer operator->() const
      {
        return m_iter->operator->();
      }

      private:
      Iterator* m_iter;
      Iterator* m_end;
    };

    /**
     * Iterates through a UTF-8 stream. The character type of the stream
     * will be interpreted as individual bytes in a UTF-8 sequence.
     */
    template <typename T>
    class UTF8Iterator : public Iterator
    {
      public:
      /**
       * Construct a UTF8Iterator.
       * @param iter The underlying iterator.
       */
      UTF8Iterator(const T& iter)
      : m_iter(iter), m_haveReadCurrent(false)
      {
      }

      /**
       * Copy a UTFIterator.
       * @param other The UTF8Iterator to copy.
       */
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
        typename T::value_type c = *m_iter;
        int toRead = 0;
        int nextShift = 0;

        //std::cerr << std::hex << (int)c << " ";

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
          std::cerr << "warning: invalid initial unicode byte" << std::endl;
        }

        ++m_iter;

        for (int i = 0; i != toRead; ++i)
        {
          c = *m_iter;
          //std::cerr << (int)c << " ";
          if ((c & 0xC0) != 0x80)
          {
            //invalid character
            std::cerr << "warning: invalid unicode byte" << std::endl;
          }
          m_value |= ((0x3F & c) << nextShift);
          nextShift -= 6;
          ++m_iter;
        }

        //std::cerr << std::endl;

        //std::cerr << "readNext() = " << m_value << std::endl;

        m_haveReadCurrent = true;
      }

      mutable T m_iter;
      mutable value_type m_value;
      mutable bool m_haveReadCurrent;
    };

    /**
     * Create a UTF8Iterator. A generator function which creates a UTF8Iterator
     * of the type of the passed parameter.
     * @param iter The underlying iterator.
     * @return A UTF8Iterator with the passed iterator as the underlying
     * iterator.
     */
    template <typename T>
    UTF8Iterator<T> makeUTF8Iterator(const T& iter)
    {
      return UTF8Iterator<T>(iter);
    }

    /**
     * Remove reference metafunction.
     * If we don't have a reference then remove reference is just the type.
     */
    template <typename T>
    struct remove_reference
    {
      typedef T type;/**<The same type.*/
    };

    /**
     * Remove reference. Removes a reference from the type.
     */
    template <typename T>
    struct remove_reference<T&>
    {
      typedef T type;/**<The type without the reference.*/
    };

    /**
     * Remove pointer. Removes a pointer from the type.
     */
    template <typename T>
    struct remove_reference<T*>
    {
      typedef T type; /**<The type without the pointer.*/
    };

    template <typename T>
    struct create_reference
    {
    };

    /**
     * Create a reference to a value. This one is for when we already have
     * a reference, so just return the value.
     */
    template <typename T>
    struct create_reference<T&>
    {
      /**
       * Creates the actual reference. Since we already have a reference just
       * return the actual value.
       */
      T& operator()(T& t)
      {
        return t;
      }
    };

    /**
     * Create a reference when we want a pointer. Takes the address of
     * the value.
     */
    template <typename T>
    struct create_reference<T*>
    {
      /**
       * Create the actual reference. We wanted a pointer reference,
       * so return the address of the value.
       */
      T* operator()(T& t)
      {
        return &t;
      }
    };

    /**
     * Dereference an object. The default is just to return the
     * object because it's not a pointer.
     */
    template <typename T>
    struct dereference
    {
      /**
       * Do the dereference. Nothing is needed here though because we
       * don't have a pointer, just return the actual value.
       */
      T operator()(T t)
      {
        return t;
      }
    };

    /**
     * Dereference an object. If the object is a pointer, return the
     * thing that is pointed to.
     */
    template <typename T>
    struct dereference<T*>
    {
      /**
       * Do the actual dereference. If we have a pointer then dereference it.
       */
      T& operator()(T* t)
      {
        return *t;
      }
    };

    /**
     * Buffer a value to return a reference.
     * A buffer for storing a value and returning a reference when 
     * the type of the value to return is different to the type 
     * we actually have.
     */
    template <typename HasType, typename NeedsType>
    struct Buffer
    {
      /**
       * Buffer a value and return the value of the new type.
       * Stores the value that we have in the type that we want, then
       * returns the stored value.
       * We need to remove the reference from the type we want so that
       * we store an actual value and not a pointer or reference.
       * Returns a pointer or a reference depending on what we needed.
       */
      NeedsType operator()(HasType v)
      {
        m_buf = dereference<HasType>()(v);
        return create_reference<NeedsType>()(m_buf);
      }

      private:
      typename remove_reference<NeedsType>::type m_buf;
    };

    /**
     * Buffer a value to return a reference.
     * This template is instantiated when the type we need is the same as the
     * type that we have. In this case just return the value.
     */
    template <typename HasType>
    struct Buffer<HasType, HasType>
    {
      /**
       * Returns the actual value. The types are the same so we just
       * return what we have.
       */
      HasType operator()(HasType v)
      {
        return v;
      }
    };

    /**
     * Dereference an iterator. Uses Buffer to store an object of the type
     * that we actually want or not store it if we have the type that we want.
     */
    template <typename HasType, typename NeedsRetType>
    struct IteratorDereference
    {
      /**
       * Dereference the iterator. Returns the value from the buffer,
       * the buffer may or may not actually store something depending
       * on the type that we need and have.
       * @param v The value that we actually have.
       * @return A reference to the value of the type that we actually need.
       */
      NeedsRetType operator()(HasType v) const
      {
        return m_buf(v);
      }

      private:
      mutable Buffer<HasType, NeedsRetType> m_buf;
    };

    /**
     * An iterator which iterates through a stream of complete unicode
     * characters. Each character in the stream is one unicode character,
     * and dereferencing the stream's iterator should return one unicode
     * character.
     */
    template <typename T>
    class UTF32Iterator : public Iterator
    {
      public:
      /**
       * Construct a UTF32Iterator from any other type of iterator.
       * The iterator must iterate through complete unicode values.
       */
      UTF32Iterator(const T& iter)
      : m_iter(iter)
      {
      }

      /**
       * Copy constructor. Construct a UTF32Iterator from another 
       * UTF32Iterator.
       * @param other The UTF32Iterator to copy.
       */
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

      Iterator& 
      operator++()
      {
        ++m_iter;
        return *this;
      }

      reference 
      operator*() const
      {
        return m_reference(*m_iter);
      }

      pointer 
      operator->() const
      {
        return m_pointer(m_iter.operator->());
      }

      private:
      T m_iter;
      IteratorDereference<typename T::reference, reference> m_reference;
      IteratorDereference<typename T::pointer, pointer> m_pointer;
    };

    /**
     * Create a UTF32Iterator. A generator function which creates a 
     * UTF32Iterator with the template parameter of the passed object.
     * @param iter The iterator to create the UTF32Iterator from.
     * @return A UTF32Iterator with the passed iterator as the underlying
     * iterator.
     */
    template <typename T>
    UTF32Iterator<T>
    makeUTF32Iterator(const T& iter)
    {
      return UTF32Iterator<T>(iter);
    }

  } //namespace Parser
} //namespace TransLucid

#endif
