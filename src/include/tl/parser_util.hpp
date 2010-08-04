/* Parser utility functions and parsers.
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

#ifndef PARSER_UTIL_HPP_INCLUDED
#define PARSER_UTIL_HPP_INCLUDED

#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/include/phoenix_bind.hpp>
#include <boost/spirit/home/phoenix/function/function.hpp>
#include <boost/spirit/include/qi_rule.hpp>
#include <boost/spirit/include/qi_grammar.hpp>
#include <boost/spirit/include/qi_char_.hpp>
#include <boost/spirit/include/qi_int.hpp>
#include <boost/spirit/include/qi_action.hpp>
#include <boost/spirit/include/qi_lexeme.hpp>
#include <boost/spirit/include/qi_char_class.hpp>

#include <gmpxx.h>
#include <tl/types.hpp>
#include <tl/parser_fwd.hpp>
#include <tl/utility.hpp>
#include <tl/charset.hpp>

namespace TransLucid
{
  namespace Parser
  {
    template <typename Iterator>
    struct SkipGrammar : public qi::grammar<Iterator>
    {

      SkipGrammar()
      : SkipGrammar::base_type(skip)
      {
        skip =
          qi::char_(' ') | '\n' | '\t'
        | (literal("//") >> *(qi::char_ - '\n') >> '\n')
        | (literal("/*") >> *(qi::char_ - literal("/*")) >> literal("*/"))
        ;
      }

      qi::rule<Iterator> skip;
    };

    struct make_u32string_impl
    {
      template <typename Arg>
      struct result
      {
        typedef std::u32string type;
      };

      template <typename Arg>
      std::u32string operator()(Arg arg1) const
      {
        return std::u32string(arg1.begin(), arg1.end());
      }

      std::u32string operator()(const u32string& arg1) const
      {
        return arg1;
      }
    };

    namespace
    {
      boost::phoenix::function<make_u32string_impl> make_u32string;
    }

    inline mpz_class
    create_mpz(char base, const u32string& s)
    {
      //0-9A-Za-z
      int actualBase;
      if (base <= '9')
      {
        actualBase = base - '0';
      }
      else if (base <= 'Z')
      {
        actualBase = base - 'A' + 10;
      }
      else
      {
        actualBase = base - 'a' + 26 + 10;
      }
      return mpz_class(u32_to_ascii(s), actualBase);
    }

    template <typename Iterator>
    class ExprGrammar;

    template <typename Iterator>
    struct integer_parser
    : public qi::grammar<Iterator, mpz_class(), qi::locals<string_type>,
                         SkipGrammar<Iterator>>
    {
      integer_parser()
        : integer_parser::base_type(integer)
      {
        namespace ph = boost::phoenix;
        using namespace qi::labels;

        integer =
          (( '0'
          >> (qi::char_('2', '9') | qi::ascii::alpha)
          >> +(qi::ascii::digit | qi::ascii::alpha)[_a += _1])
            [
               _val = ph::bind(&create_mpz, _1, make_u32string(_a))
            ]
          | qi::int_ [qi::_val = qi::_1])
        ;
      }

      qi::rule<
        Iterator,
        mpz_class(),
        qi::locals<string_type>,
        SkipGrammar<Iterator>
      > integer;
    };

    template <typename Iterator>
    class escaped_string_parser : public qi::grammar<Iterator, string_type()>
    {
      public:
      escaped_string_parser()
        : escaped_string_parser::base_type(string)
      {
        using namespace qi::labels;

        string =
          literal('<')
            >> (
                *(escaped | (qi::unicode::char_ - literal('>')))
                 [
                   _val += _1
                 ]
               )
            >> literal('>')
        ;

        escaped =
          ('\\' >> qi::unicode::char_)
          [_val = _1]
        ;
      }

      private:
      qi::rule<Iterator, string_type()>
        string
      ;

      qi::rule<Iterator, char_type()>
        escaped
      ;
    };

    template <typename Iterator>
    struct ident_parser
    : public qi::grammar<Iterator, string_type(), SkipGrammar<Iterator>>
    {
      ident_parser()
      : ident_parser::base_type(ident)
      {
        ident %= qi::unicode::alpha >> *(qi::unicode::alnum | '_')
        ;
      }

      qi::rule<Iterator, string_type(), SkipGrammar<Iterator>> ident;
    };

    //inline const
    //string_type& getDelimiterType(const Delimiter& d)
    //{
    //  return d.type;
    //}

    //TODO this may be useful for handling errors
    #if 0
    struct handle_expr_error
    {
      template <class ScannerT, class ErrorT>
      Spirit::error_status<>
      operator()(ScannerT const& scan, ErrorT error) const
      {

        //for some reason spirit resets the scanner start position
        //after a retry fails so we need to be past the error before
        //we can keep going
        while (scan.first != error.where)
        {
          ++scan;
        }

        //look for a ;;
        bool found = false;
        bool firstFound = false;
        while (!found && !scan.at_end())
        {
          if (!firstFound)
          {
            if (*scan == ';')
            {
              firstFound = true;
            }
            ++scan;
          }
          else
          {
            if (*scan == ';')
            {
              found = true;
            }
            else
            {
              firstFound = false;
              ++scan;
            }
          }
        }

        //if we get to the end of input without finding a ;; then print
        //the error message
        if (!scan.at_end() && found)
        {
          printErrorMessage(error.where.get_position(), error.descriptor);
        }

        if (!scan.at_end())
        {
          ++scan;
        }

        Spirit::error_status<>::result_t result =
          !found && scan.at_end() ?
          Spirit::error_status<>::fail :
          Spirit::error_status<>::retry;

        return Spirit::error_status<>(result);
      }
    };
    #endif
  }
}

#endif // PARSER_UTIL_HPP_INCLUDED
