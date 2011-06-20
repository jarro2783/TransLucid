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
    class InstantGrammar : public qi::grammar<Iterator, void(InstantFunctor&)>
    {
      public:
      template <typename TokenDef>
      InstantGrammar
      (
        TokenDef& tok,
        LineGrammar<Iterator>& line 
      )
      : InstantGrammar::base_type(r_instants)
      , g_line(line)
      {
        using namespace qi::labels;
        r_instant = (*g_line >> tok.dollar_)
        [
          boost::phoenix::bind(
            &InstantGrammar<Iterator>::call_instant_functor, _r1)
        ];

        r_instants = r_instant(_r1) 
          >> *(tok.dollar_ >> tok.dollar_ >> r_instant(_r1));
      }

      qi::rule<Iterator, void(InstantFunctor&)> r_instants;
      qi::rule<Iterator, void(InstantFunctor&)> r_instant;
      LineGrammar<Iterator>& g_line;

      static void
      call_instant_functor(const InstantFunctor& i)
      {
        i(Instant());
      }
    };
  }
}

#endif
