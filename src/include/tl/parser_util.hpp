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

#include <gmpxx.h>
#include <tl/types.hpp>
#include <tl/parser_fwd.hpp>
#include <tl/utility.hpp>
#include <tl/charset.hpp>

namespace TransLucid
{
  namespace Parser
  {
    typedef std::unordered_map<u32string, Tree::Expr> ReservedIdentifierMap;
    //determines if an identifier is a dimension, some named constant, or
    //just an identifier
    Tree::Expr
    construct_identifier
    (
      const u32string& id,
      const ReservedIdentifierMap& ids
    );

    //initialises the reserved identifier map, this might need to change
    //in the future too
    ReservedIdentifierMap
    init_reserved_identifiers();

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

#ifdef BOOST_SPIRIT_DEBUG
namespace std
{
  inline
  ostream&
  operator<<(ostream& os, 
    const pair<TransLucid::u32string, TransLucid::u32string>& p)
  {
    os << "pair: \"" << p.first << "\", \"" << p.second << "\"";
    return os;
  }
}
#endif

#endif // PARSER_UTIL_HPP_INCLUDED
