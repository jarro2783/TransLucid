#ifndef HEADER_TYPE_HPP_INCLUDED
#define HEADER_TYPE_HPP_INCLUDED

#if 0

#include <tl/types.hpp>
#include <tl/parser_fwd.hpp>
#include <tl/interpreter.hpp>

namespace TransLucid
{
  class HeaderType : public TypedValueBase
  {
    public:

    enum Input
    {
      DIRECT,
      FILE
    };

    size_t
    hash() const;

    const Parser::Header&
    header() const
    {
      return m_header;
    }

    bool
    operator==(const HeaderType& rhs) const;

    void
    parseString(const u32string& s, const Tuple& c, Interpreter& i);

    void
    parseFile(const u32string& file, const Tuple& c, Interpreter& i);

    bool
    operator<(const HeaderType& rhs) const
    {
      return hash() < rhs.hash();
    }

    private:
    Parser::Header m_header;
  };

  namespace HeaderImp
  {
    template <HeaderType::Input H>
    struct parser
    {
    };

    template <>
    struct parser<HeaderType::DIRECT>
    {
      void
      operator()
      (
        HeaderType& h,
        const u32string& s,
        const Tuple& c,
        Interpreter& i
      ) const
      {
        h.parseString(s, c, i);
      }
    };

    template <>
    struct parser<HeaderType::FILE>
    {
      void
      operator()
      (
        HeaderType& h,
        const u32string& s,
        const Tuple& c,
        Interpreter& i
      ) const
      {
         h.parseFile(s, c, i);
      }
    };

    template <HeaderType::Input H>
    struct name
    {
    };

    template <>
    struct name<HeaderType::DIRECT>
    {
      const char*
      operator()()
      {
         return "header";
      }
    };

    template <>
    struct name<HeaderType::FILE>
    {
      const char*
      operator()()
      {
         return "headerfile";
      }
    };
  }
};

#endif

#endif // HEADER_TYPE_HPP_INCLUDED
