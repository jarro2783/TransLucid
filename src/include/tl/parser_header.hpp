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

#ifndef PARSER_HEADER_INCLUDED
#define PARSER_HEADER_INCLUDED

/**
 * @file parser_header.hpp
 * Utility functions for the TransLucid header.
 */

#include <tl/ast.hpp>

namespace TransLucid
{

  namespace Parser
  {

    typedef std::map<u32string, Tree::UnaryOperator> unary_symbols;
    typedef std::map<u32string, Tree::BinaryOperator> binary_symbols;
    typedef std::set<u32string> dimension_set;

    struct Header
    {
      /**
       * Construct a header. Adds the default dimension symbols.
       */
      Header();

      dimension_set dimension_symbols;/**<Dimensions.*/
      dimension_set system_dimension_symbols;

      unary_symbols prefix_op_symbols;/**<Prefix operations.*/
      unary_symbols postfix_op_symbols;/**<Postfix operations.*/

      std::vector<u32string> libraries; /**<Libraries not yet loaded.*/
      std::vector<u32string> loaded_libraries; /**<The loaded libraries.*/

      binary_symbols binary_op_symbols; 
      /**<Binary operators.*/
    };

    /**
     * Adds a dimension symbol to the header.
     * @param h The header to add the symbol to.
     * @param name The name of the dimension symbol.
     */
    inline void
    addDimensionSymbol(Header& h, const u32string& name)
    {
      h.dimension_symbols.insert(name);
    }

    /**
     * Removes a dimension symbol from a header.
     * @param h The header to remove the symbol from.
     * @param name The name of the dimension symbol.
     */
    inline void
    removeDimensionSymbol(Header& h, const u32string& name)
    {
      h.dimension_symbols.erase(name);
    }

     /**
     * Add a binary operation symbol to a header.
     * @param h The header to add.
     * @param symbol The symbol which represents the operator.
     * @param opName The name of the operation.
     * @param assoc The associativity.
     * @param precedence The precedence.
     */
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
      if (h.binary_op_symbols.find(symbol) != h.binary_op_symbols.end())
      {
        throw ParseError(U"Existing binary operator");
      }
      //std::cerr << "adding " << 
      //  utf32_to_utf8(u32string(symbol.begin(), symbol.end())) << std::endl;
      h.binary_op_symbols.insert(std::make_pair
      (
        symbol,
        Tree::BinaryOperator
        (
          assoc,
          opName,
          symbol,
          precedence
        )
      ));
    }

    /**
     * Add a unary operation symbol to a header.
     * @param header The header to add to.
     * @param type The type of the operation. UNARY_PREFIX or UNARY_POSTFIX.
     * @param symbol The symbol to add.
     * @param op The name of the operation.
     */
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

      if (type == Tree::UNARY_PREFIX)
      {
        if (header.prefix_op_symbols.find(symbol) 
            != header.prefix_op_symbols.end()) 
        {
          throw ParseError(U"Existing prefix symbol");
        }
        header.prefix_op_symbols.insert(std::make_pair
        (
          symbol,
          opinfo
        ));
      }
      else
      {
        if (header.postfix_op_symbols.find(symbol) 
            != header.postfix_op_symbols.end()) 
        {
          throw ParseError(U"Existing postfix symbol");
        }

        header.postfix_op_symbols.insert(std::make_pair
        (
          symbol,
          opinfo
        ));
      }
    }

    /**
     * Add a library to a header.
     * @param header The header to add to.
     * @param library The library name to add.
     */
    inline void
    addLibrary
    (
      Header& header,
      const u32string& library
    )
    {
      header.libraries.push_back(library);
    }

    /**
     * Print a header.
     * @param os The output stream.
     * @param h The header to print.
     * @return os.
     */
    std::ostream& 
    operator<<(std::ostream& os, const Header& h);
  } //namespace Parser
} //namespace TransLucid

#endif //PARSER_HEADER_UTIL_INCLUDED
