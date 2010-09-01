/* Parser utility functions for header.
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

#ifndef PARSER_HEADER_UTIL_INCLUDED
#define PARSER_HEADER_UTIL_INCLUDED

/**
 * @file parser_header_util.hpp
 * Utility functions for the TransLucid header.
 */

#include <tl/parser_fwd.hpp>

namespace TransLucid
{

  namespace Parser
  {

    typedef qi::symbols<char_type, Tree::UnaryOperator> unary_symbols;
    typedef qi::symbols<char_type, Tree::BinaryOperator> binary_symbols;
    typedef qi::symbols<char_type, Delimiter> delimiter_symbols;

    struct Header
    {
      Header();

      symbols_t dimension_symbols;

      binary_symbols binary_op_symbols;

      unary_symbols prefix_op_symbols;
      unary_symbols postfix_op_symbols;

      delimiter_symbols delimiter_start_symbols;

      std::vector<u32string> libraries;
    };

    inline void
    setEndDelimiter(Delimiter& d, wchar_t& end)
    {
      end = d.end;
    }

    inline void
    addDimensionSymbol(Header& h, const u32string& name)
    {
      string_type wsname(name.begin(), name.end());
      h.dimension_symbols.add(wsname.c_str(), name);
    }

    inline void
    removeDimensionSymbol(Header& h, const u32string& name)
    {
      h.dimension_symbols.remove(name);
    }

    inline void
    addBinaryOpSymbol
    (
      Header& h,
      const u32string& symbol,
      const u32string& opName,
      Tree::InfixAssoc assoc,
      const mpz_class& precedence
    )
    {
      auto usymbol = to_unsigned_u32string(symbol);
      if (h.binary_op_symbols.find(usymbol.c_str()) != 0) 
      {
        throw ParseError(U"Existing binary operator");
      }
      std::cerr << "adding " << 
        utf32_to_utf8(u32string(symbol.begin(), symbol.end())) << std::endl;
      h.binary_op_symbols.add
      (
        usymbol.c_str(),
        Tree::BinaryOperator
        (
          assoc,
          opName,
          symbol,
          precedence
        )
      );
    }

    inline void
    addDelimiterSymbol
    (
      Header& header,
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
      Header& header,
      Tree::UnaryType type,
      const u32string& symbol,
      const u32string& op
    )
    {
      Tree::UnaryOperator opinfo
      (
        op,
        symbol,
        type
      );

      auto usymbol = to_unsigned_u32string(symbol);

      if (type == Tree::UNARY_PREFIX)
      {
        if (header.prefix_op_symbols.find(symbol) != 0) 
        {
          throw ParseError(U"Existing prefix symbol");
        }
        header.prefix_op_symbols.add
        (
          usymbol.c_str(),
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
          usymbol.c_str(),
          opinfo
        );
      }
    }

    inline void
    addLibrary
    (
      Header& header,
      const u32string& library
    )
    {
      header.libraries.push_back(library);
    }

    inline std::ostream& 
    operator<<(std::ostream& os, const Header& h) 
    {
      os << "Parser::Header";
      return os;
    }
  } //namespace Parser
} //namespace TransLucid

#endif //PARSER_HEADER_UTIL_INCLUDED
