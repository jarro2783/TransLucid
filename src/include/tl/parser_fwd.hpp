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

#ifndef PARSER_FWD_HPP_INCLUDED
#define PARSER_FWD_HPP_INCLUDED

#include <tuple>
//#include <boost/spirit/include/qi_core.hpp>
#include <boost/spirit/include/qi_symbols.hpp>
#include <vector>
#include <tl/types.hpp>
#include <tl/equation.hpp>
//#include <boost/fusion/include/adapt_struct.hpp>
#include <tl/ast.hpp>
#include <tl/utility.hpp>
#include <tl/parser_iterator.hpp>

inline std::ostream&
operator<<(std::ostream& os, char32_t c)
{
  os << TransLucid::utf32_to_utf8(TransLucid::u32string(1, c));
  return os;
}

namespace TransLucid
{
  /**
   * The parser namespace. All the parsing is in here.
   */
  namespace Parser
  {
    enum ParseErrorType
    {
      error_expected_fi,
      error_expected_else,
      error_expected_colon,
      error_expected_dbl_semi,
      error_expected_primary,
      error_expected_close_paren,
      error_expected_then,
      error_expected_close_bracket,
      //an error that hasn't been specifically dealt with
      error_unknown
    };

    namespace qi = boost::spirit::qi;
    namespace fusion = boost::fusion;

    //typedef std::basic_string<wchar_t> string_type;
    typedef std::basic_string<unsigned int> string_type;
    typedef unsigned int char_type;
    typedef qi::symbols<char_type, std::u32string> symbols_t;

    struct Delimiter
    {
      Delimiter() = default;

      Delimiter(
        const u32string& type,
        char_type start,
        char_type end)
      : type(type), start(start), end(end)
      {}

      bool
      operator==(const Delimiter& rhs) const
      {
        return type == rhs.type && start == rhs.start && end == rhs.end;
      }

      u32string type;
      char_type start;
      char_type end;
    };

    inline std::ostream&
    operator<<(std::ostream& os, const Delimiter& d)
    {
      os << "delimiter(" << d.type << ")";
      return os;
    }

    typedef U32Iterator iterator_t;
  }
}

#if 0
namespace std
{
  inline ostream&
  operator<<(ostream& os, const TransLucid::Parser::string_type& s)
  {
    os << TransLucid::utf32_to_utf8(TransLucid::to_u32string(s));
    return os;
  }
}
#endif

#endif // PARSER_FWD_HPP_INCLUDED
