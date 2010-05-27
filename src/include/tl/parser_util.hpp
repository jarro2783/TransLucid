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

#include <gmpxx.h>
#include <tl/types.hpp>
#include <tl/parser_fwd.hpp>
#include <tl/utility.hpp>

namespace TransLucid
{
  namespace Parser
  {
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
          '<'
            >> (
                *(escaped | (qi::standard_wide::char_ - '>'))
                 [
                   _val += _1
                 ]
               )
            >> '>'
        ;

        escaped =
          ('\\' >> qi::standard_wide::char_)
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
        ident %= qi::ascii::alpha >> *(qi::ascii::alnum | '_')
        ;
      }

      qi::rule<Iterator, string_type(), SkipGrammar<Iterator>> ident;
    };

    inline void
    setEndDelimiter(Delimiter& d, wchar_t& end)
    {
      end = d.end;
    }

    inline void
    addDimensionSymbol(HeaderStruct& h, const u32string& name)
    {
      string_type wsname(name.begin(), name.end());
      h.dimension_symbols.add(wsname.c_str(), name);
    }

    inline void
    addBinaryOpSymbol
    (
      HeaderStruct& h,
      const string_type& symbol,
      const string_type& opName,
      Tree::InfixAssoc assoc,
      const mpz_class& precedence
    )
    {
      if (h.binary_op_symbols.find(symbol.c_str()) != 0) 
      {
        throw ParseError(U"Existing binary operator");
      }
      std::cerr << "adding " << 
        utf32_to_utf8(u32string(symbol.begin(), symbol.end())) << std::endl;
      h.binary_op_symbols.add
      (
        symbol.c_str(),
        Tree::BinaryOperator
        (
          assoc,
          to_u32string(opName),
          to_u32string(symbol),
          precedence
        )
      );
    }

    inline void
    addDelimiterSymbol
    (
      HeaderStruct& header,
      const u32string& type,
      char32_t open,
      char32_t close
    )
    {
      string_type open_string(1, open);
      if (header.delimiter_start_symbols.find(open_string) != 0) 
      {
        throw ParseError(U"Existing delimiter");
      }
      header.delimiter_start_symbols.add
      (
        open_string.c_str(),
        Delimiter(type, (char_type)open, (char_type)close)
      );
    }

    inline void
    addUnaryOpSymbol
    (
      HeaderStruct& header,
      Tree::UnaryType type,
      const string_type& symbol,
      const string_type& op
    )
    {
      Tree::UnaryOperator opinfo
      (
        to_u32string(op),
        to_u32string(symbol),
        type
      );

      if (type == Tree::UNARY_PREFIX)
      {
        if (header.prefix_op_symbols.find(symbol) != 0) 
        {
          throw ParseError(U"Existing prefix symbol");
        }
        header.prefix_op_symbols.add
        (
          symbol.c_str(),
          opinfo
        );
      }
      else
      {
        if (header.postfix_op_symbols.find(symbol) != 0) 
        {
          throw ParseError(U"Existing postfix symbol");
        }

        header.postfix_op_symbols.add
        (
          symbol.c_str(),
          opinfo
        );
      }
    }

    inline void
    addLibrary
    (
      HeaderStruct& header,
      const u32string& library
    )
    {
    }

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
