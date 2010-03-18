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
#include <boost/spirit/include/qi_core.hpp>
#include <vector>
#include <boost/spirit/include/qi_rule.hpp>
#include <boost/spirit/include/qi_grammar.hpp>
#include <tl/types.hpp>
#include <tl/equation.hpp>
#include <boost/fusion/include/adapt_struct.hpp>
#include <tl/ast.hpp>
#include <tl/utility.hpp>

inline std::ostream&
operator<<(std::ostream& os, char32_t c)
{
  os << TransLucid::utf32_to_utf8(TransLucid::u32string(1, c));
  return os;
}

namespace TransLucid
{
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

    typedef std::basic_string<wchar_t> string_type;
    typedef wchar_t char_type;
    typedef qi::symbols<char_type, std::u32string> symbols_t;

    typedef qi::standard_wide::space_type skip;

    template <typename Iterator>
    struct SkipGrammar : public qi::grammar<Iterator>
    {

      SkipGrammar()
      : SkipGrammar::base_type(skip)
      {
        skip =
          qi::char_(' ') | '\n' | '\t'
        | "//"
        | ("/*" >> *(qi::char_ - "/*") >> "*/")
        ;
      }

      qi::rule<Iterator> skip;
    };

    template <typename Iterator>
    class ExprGrammar;

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

    typedef qi::symbols<char_type, Tree::UnaryOperation> unary_symbols;
    typedef qi::symbols<char_type, Tree::BinaryOperation> binary_symbols;
    typedef qi::symbols<char_type, Delimiter> delimiter_symbols;

    struct Header
    {
      symbols_t dimension_symbols;

      binary_symbols binary_op_symbols;

      unary_symbols prefix_op_symbols;
      unary_symbols postfix_op_symbols;

      delimiter_symbols delimiter_start_symbols;

      std::vector<u32string> libraries;
    };

    typedef string_type::const_iterator iterator_t;
  }
}

namespace std
{
  inline ostream&
  operator<<(ostream& os, const TransLucid::Parser::string_type& s)
  {
    os << TransLucid::utf32_to_utf8(TransLucid::to_u32string(s));
    return os;
  }
}

#endif // PARSER_FWD_HPP_INCLUDED
