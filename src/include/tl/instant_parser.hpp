/* Parses a whole instant.
   Copyright (C) 2011 Jarryd Beck and John Plaice

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

#ifndef TL_INSTANT_PARSER_HPP_INCLUDED
#define TL_INSTANT_PARSER_HPP_INCLUDED

#include <boost/spirit/include/qi_grammar.hpp>
#include <boost/spirit/include/qi_rule.hpp>

#include <tl/line_parser.hpp>

namespace TransLucid
{
  namespace Parser
  {
    template <typename Iterator>
    class InstantGrammar : public qi::grammar<Iterator>
    {
      public:
      template <typename TokenDef>
      InstantGrammar(TokenDef& tok)
      : InstantGrammar::base_type(r_instant)
      , g_line(tok)
      {
        r_instant = *g_line >> tok.dbldollar_;
      }

      qi::rule<Iterator> r_instant;
      LineGrammar<Iterator> g_line;
    };
  }
}

#endif
