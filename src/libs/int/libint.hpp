#ifndef INTLIB_TYPEMANAGER_HPP_INCLUDED
#define INTLIB_TYPEMANAGER_HPP_INCLUDED

#include <tl/library.hpp>
#include <tl/types.hpp>
#include <boost/lexical_cast.hpp>

namespace IntLib
{

  template <class T>
  class Int : public TransLucid::TypedValueBase
  {
    public:
    Int(T value)
    : m_value(value)
    {}

    void
    print(std::ostream& os, const TransLucid::Tuple& context) const;

    bool
    operator==(const Int& rhs) const
    {
      return m_value == rhs.m_value;
    }

    const T&
    value() const
    {
      return m_value;
    }

    //static Int<T>
    //parse
    //(
    //  const TransLucid::u32string& text,
    //  const TransLucid::Tuple& context,
    //  const TransLucid::Interpreter& i
    //)
    //{
    //  return Int<T>(boost::lexical_cast<T>(text));
    //}

    size_t
    hash() const
    {
      boost::hash<T> hasher;
      return hasher(m_value);
    }

    bool
    operator<(const Int<T>& rhs) const
    {
      return m_value < rhs.m_value;
    }

    private:
    T m_value;
  };

  template <class T>
  size_t
  hash_value(const Int<T>& v)
  {
    boost::hash<T> hasher;
    return hasher(v.value());
  }
};

#endif // TYPEMANAGER_HPP_INCLUDED
