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

/**
 * @file parser_util.hpp
 * Parsing utility functions and definitions.
 */

#include <unordered_map>

#include <tl/types.hpp>
#include <tl/utility.hpp>
#include <tl/charset.hpp>
#include <tl/ast.hpp>
//#include <tl/parser_defs.hpp>
#include <tl/parser_header.hpp>
#include <tl/system.hpp>

namespace TransLucid
{
  namespace Parser
  {
    /**
     * Constructs an identifier.
     * determines if an identifier is a dimension, some named constant, or
     * just an identifier.
     * @param id The identifier name.
     * @param ids The reserved identifiers.
     * @return An expression node which represents the identifier.
     */
    Tree::Expr
    construct_identifier
    (
      const u32string& id,
      const System::IdentifierLookup& ids,
      Context*& context,
      dimension_index nameDim
    );
  }
}
#endif // PARSER_UTIL_HPP_INCLUDED
