/* Parser iterator.
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

#ifndef PARSER_ITERATOR_HPP_INCLUDED
#define PARSER_ITERATOR_HPP_INCLUDED

#ifdef ITERATOR_DEBUG
#include <iostream>
#endif

#include <iterator>
#include <memory>
#include <typeinfo>

#include <tl/charset.hpp>

//define this to get lots of debugging
//#define ITERATOR_DEBUG

namespace TransLucid
{
  namespace Parser
  {
    struct iterator_traits :
      public std::iterator
      <
        std::forward_iterator_tag,
        char32_t
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
      : m_iter(nullptr)
      {
        #ifdef ITERATOR_DEBUG
        std::cerr << this << ": U32Iterator()" << std::endl;
        #endif
      }

      ~U32Iterator()
      {
        delete m_iter;
      }

      /**
       * Construct a U32Iterator.
       * We need to know where the end of the stream is to decide when we
       * have reached the end.
       */
      U32Iterator(const Iterator& i)
      : m_iter(i.clone())
      {
        #ifdef ITERATOR_DEBUG
        std::cerr << this << ": U32Iterator(i, end)" << std::endl;
        #endif
      }

      /**
       * Copy construct a U32Iterator.
       * @param other The U32Iterator to make a copy of.
       */
      U32Iterator(const U32Iterator& other)
      : m_iter(other.m_iter != nullptr ? other.m_iter->clone() : nullptr)
      {
        #ifdef ITERATOR_DEBUG
        std::cerr << this << " U32Iterator(*" << &other << ")" << std::endl;
        #endif
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
        #ifdef ITERATOR_DEBUG
        std::cerr << this << " = " << &rhs << std::endl;
        #endif
        if (this != &rhs)
        {
          Iterator* iter_copy = nullptr;
          Iterator* end_copy = nullptr;

          try
          {
            iter_copy = rhs.m_iter != nullptr ? rhs.m_iter->clone() : nullptr;

            delete m_iter;

            m_iter = iter_copy;
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
        return (m_iter == nullptr && rhs.m_iter == nullptr)
          || (m_iter != nullptr || rhs.m_iter != nullptr || 
              *m_iter == *rhs.m_iter)
        ;
        #if 0
        #ifdef ITERATOR_DEBUG
        std::cerr << this << " == " << &rhs;
        #endif
        bool lhs_end = false;
        bool rhs_end = false;

        if (m_iter == nullptr || *m_iter == *m_end)
        {
          lhs_end = true;
        }

        if (rhs.m_iter == nullptr || *rhs.m_iter == *rhs.m_end)
        {
          rhs_end = true;
        }

        if (!lhs_end && !rhs_end)
        {
          #ifdef ITERATOR_DEBUG
          std::cerr << " " << (*m_iter == *rhs.m_iter) << std::endl;
          #endif
          return *m_iter == *rhs.m_iter;
        }
        else
        {
          #ifdef ITERATOR_DEBUG
          std::cerr << " " << (lhs_end == rhs_end) << std::endl;
          #endif
          return lhs_end == rhs_end;
        }
        #endif
      }

      /**
       * Equality test.
       * @param rhs The other iterator to compare to.
       * @return !(*this == rhs)
       * @see U32Iterator::operator==
       */
      bool operator!=(const U32Iterator& rhs) const
      {
        #ifdef ITERATOR_DEBUG
        std::cerr << this << ": !=" << &rhs << std::endl;
        #endif
        return !this->operator==(rhs);
      }

      /**
       * Pre-increment. Increments the iterator.
       * @return The value after being iterated.
       */
      U32Iterator& operator++()
      {
        #ifdef ITERATOR_DEBUG
        std::cerr << this << ": ++" << std::endl;
        #endif
        ++*m_iter;
        //std::cerr << "++ " << u32string(1, operator*()) << std::endl;
        return *this;
      }

      /**
       * Post-increment. Increments the iterator.
       * @return The value before being iterated.
       */
      const U32Iterator operator++(int)
      {
        #ifdef ITERATOR_DEBUG
        std::cerr << this << ": i++" << std::endl;
        #endif
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
        #ifdef ITERATOR_DEBUG
        std::cerr << this << ": *" << std::endl;
        char32_t value = **m_iter;
        std::cerr << u32string(U"> ") + u32string(1, value) << std::endl;
        #endif
        return **m_iter;
      }

      /**
       * Dereference operator. Returns a pointer.
       * @return A pointer to the pointed to value.
       */
      pointer operator->() const
      {
        #ifdef ITERATOR_DEBUG
        std::cerr << this << ": ->" << std::endl;
        #endif
        return m_iter->operator->();
      }

      private:
      Iterator* m_iter;
    };

    namespace detail
    {
      template <typename T>
      struct UTF8Iterator_shared
      {
        public:
        UTF8Iterator_shared(const T& iter)
        : m_iter_actual(iter)
        , m_iter(iter)
        , m_haveRead(false)
        {
        }

        UTF8Iterator_shared(const UTF8Iterator_shared& other)
        : m_iter_actual(other.m_iter_actual)
        , m_iter(other.m_iter)
        , m_haveRead(other.m_haveRead)
        , m_value(other.m_value)
        {
        }

        bool
        operator==(const UTF8Iterator_shared<T>& rhs) const
        {
          return m_iter_actual == rhs.m_iter_actual;
        }

        void 
        increment()
        {
          if (!m_haveRead)
          {
            readNext();
          }

          ++m_iter;
          m_iter_actual = m_iter;
          m_haveRead = false;
        }

        char32_t&
        get()
        {
          if (!m_haveRead)
          {
            readNext();
            m_haveRead = true;
          }
          return m_value;
        }

        void readNext()
        {
          m_iter_actual = m_iter;
          //std::cerr << "readNext() ";
          m_value = 0;
          typename T::value_type c = *m_iter;
          int toRead = 0;
          int nextShift = 0;

          //std::cerr << std::hex << 
          //  static_cast<unsigned int>(static_cast<unsigned char>(c)) << " ";

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
            //std::cerr << "warning: invalid initial unicode byte" << std::endl;
          }

          //std::cerr << "reading " << toRead << "bytes" << std::endl;
          for (int i = 0; i != toRead; ++i)
          {
            ++m_iter;
            c = *m_iter;
            //std::cerr << (int)c << " ";
            if ((c & 0xC0) != 0x80)
            {
              //invalid character
              //std::cerr << "warning: invalid unicode byte" << std::endl;
            }
            m_value |= ((0x3F & c) << nextShift);
            nextShift -= 6;
          }
        }

        private:
        T m_iter_actual;
        T m_iter;
        bool m_haveRead;
        char32_t m_value;
      };
    }

    /**
     * Iterates through a UTF-8 stream. The character type of the stream
     * will be interpreted as individual bytes in a UTF-8 sequence.
     */
    template <typename T>
    class UTF8Iterator : public Iterator
    {
      public:

      //T must be a forward iterator
      //static_assert(is_forward_iterator<T>::value == 1, 
      //  "The UTF8 underlying iterator must be a forward iterator.");

      #ifndef DOXYGEN_SHOULD_SKIP_THIS
      __glibcxx_class_requires(_ForwardIteratorConcept<T>, 
        "Need forward iterator")
      #endif

      /**
       * Construct a UTF8Iterator.
       * @param iter The underlying iterator.
       */
      UTF8Iterator(const T& iter)
      : m_data(new detail::UTF8Iterator_shared<T>(iter))
      {
      }

      /**
       * Copy a UTFIterator.
       * @param other The UTF8Iterator to copy.
       */
      UTF8Iterator(const UTF8Iterator& other)
      : m_data(other.m_data)
      {
      }

      UTF8Iterator* clone() const
      {
        return new UTF8Iterator(*this);
      }

      bool operator==(const Iterator& rhs) const
      {
        const UTF8Iterator* crhs = dynamic_cast<const UTF8Iterator*>(&rhs);
        if (crhs)
        {
          return *m_data == *(*crhs).m_data;
        }
        else
        {
          return false;
        }
      }

      Iterator& operator++() 
      {
        if (m_data.use_count() != 1)
        {
          m_data.reset(new detail::UTF8Iterator_shared<T>(*m_data.get()));
        }

        m_data->increment();

        return *this;
      }

      reference operator*() const
      {
        return m_data->get();
      }

      pointer operator->() const
      {
        return &m_data->get();
      }

      private:
      std::shared_ptr<detail::UTF8Iterator_shared<T>> m_data;
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
        const UTF32Iterator<T>* rhs_cast = 
            dynamic_cast<const UTF32Iterator<T>*>(&rhs);
        if (rhs_cast)
        {
          return m_iter == rhs_cast->m_iter;
        }
        else
        {
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

    template <typename T>
    class PositionIterator : 
      public std::iterator<std::bidirectional_iterator_tag, char32_t>
    {
      public:
      PositionIterator() = default;
      PositionIterator(const T& iter) 
      : m_iter(iter), m_line(0), m_char(0) {}

      template <typename String>
      PositionIterator
      (
        const T& iter, 
        String&& file,
        int line, 
        int c
      ) 
      : m_iter(iter)
      , m_file(std::forward<String>(file))
      , m_line(line)
      , m_char(c)
      {}
      
      template <typename String>
      PositionIterator
      (
        const T& iter, 
        String&& file
      )
      : m_iter(iter), m_file(std::forward<String>(file))
      , m_line(0), m_char(0)
      {
      }


      bool
      operator==(const PositionIterator& rhs) const
      {
        return m_iter == rhs.m_iter;
      }

      bool 
      operator!=(const PositionIterator& rhs) const
      {
        return !(*this == rhs);
      }

      PositionIterator&
      operator++()
      {
        ++m_iter;

        char32_t c = *m_iter;
        if (c == '\n')
        {
          m_char = 0;
          ++m_line;
        }
        else
        {
          ++m_char;
        }

        return *this;
      }

      PositionIterator
      operator++(int)
      {
        PositionIterator old(*this);
        ++(*this);
        return old;
      }

      char32_t
      operator*()
      {
        return *m_iter;
      }

      int
      getLine() const
      {
        return m_line;
      }

      int
      getChar() const
      {
        return m_char;
      }

      const u32string&
      getFile() const
      {
        return m_file;
      }

      const T&
      getIterator() const
      {
        return m_iter;
      }

      private:
      T m_iter;

      u32string m_file;
      int m_line;
      int m_char;
    };

    typedef PositionIterator<U32Iterator> StreamPosIterator;
  } //namespace Parser
} //namespace TransLucid

#endif
