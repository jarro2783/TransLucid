/* Header type, for customising the parser at runtime.
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

#ifndef HEADER_HPP_INCLUDED
#define HEADER_HPP_INCLUDED

// TODO: This file is under construction.  It is not junk.

#if 0

#include <tl/types.hpp>
#include <tl/parser_fwd.hpp>
#include <tl/system.hpp>

namespace TransLucid
{
  class Header : public TypedValue
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
    operator==(const Header& rhs) const;

    void
    parseString(const u32string& s, const Tuple& c, SystemHD& i);

    void
    parseFile(const u32string& file, const Tuple& c, SystemHD& i);

    bool
    operator<(const Header& rhs) const
    {
      return hash() < rhs.hash();
    }

    private:
    Parser::Header m_header;
  };

  namespace HeaderImp
  {
    template <Header::Input H>
    struct parser
    {
    };

    template <>
    struct parser<Header::DIRECT>
    {
      void
      operator()
      (
        Header& h,
        const u32string& s,
        const Tuple& c,
        SystemHD& i
      ) const
      {
        h.parseString(s, c, i);
      }
    };

    template <>
    struct parser<Header::FILE>
    {
      void
      operator()
      (
        Header& h,
        const u32string& s,
        const Tuple& c,
        SystemHD& i
      ) const
      {
         h.parseFile(s, c, i);
      }
    };

    template <Header::Input H>
    struct name
    {
    };

    template <>
    struct name<Header::DIRECT>
    {
      const char*
      operator()()
      {
         return "header";
      }
    };

    template <>
    struct name<Header::FILE>
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

#endif // HEADER_HPP_INCLUDED