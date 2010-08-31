/* Equation Parser.
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

#ifndef EQUATION_PARSER_HPP_INCLUDED
#define EQUATION_PARSER_HPP_INCLUDED

#include <tl/parser_util.hpp>
#include <tl/parser_fwd.hpp>
#include <boost/spirit/include/qi_auxiliary.hpp>
#include <boost/spirit/home/phoenix/object/construct.hpp>
#include <boost/spirit/include/qi_core.hpp>

namespace TransLucid
{
  namespace Parser
  {
    namespace ph = boost::phoenix;
    using namespace ph;

    template <typename Iterator>
    class EquationGrammar
    : public qi::grammar<Iterator, ParsedEquation(),qi::locals<string_type>,
      SkipGrammar<Iterator>>
    {
      public:

      EquationGrammar()
      : EquationGrammar::base_type(equation)
      {
        using namespace qi::labels;

        equation =
          (
              ident[_a = _1]
           >> guard
           >> boolean
           >> qi::lit('=')
           >> expr
          )
          [
            _val = construct<ParsedEquation>(_a, _2, _3, _4)
          ]
        ;

        guard =
          ( qi::lit('|') >> context_perturb[_val = _1])
        | qi::eps [_val = construct<Tree::nil>()]
        ;

        boolean =
          ( qi::lit('&') >> expr[_val = _1])
        | qi::eps [_val = construct<Tree::nil>()]
        ;

        BOOST_SPIRIT_DEBUG_NODE(boolean);
        BOOST_SPIRIT_DEBUG_NODE(guard);
        BOOST_SPIRIT_DEBUG_NODE(equation);
        BOOST_SPIRIT_DEBUG_NODE(expr);
        BOOST_SPIRIT_DEBUG_NODE(context_perturb);
      }

      template <typename T>
      void
      set_tuple(const T& t)
      {
        using namespace qi::labels;
        context_perturb = t[_val = _1];
      }

      template <typename T>
      void
      set_expr(const T& t)
      {
        expr %= t;
      }

      private:

      qi::rule<Iterator, ParsedEquation(), qi::locals<string_type>,
               SkipGrammar<Iterator>>
        equation
      ;

      qi::rule<Iterator, Tree::Expr(), SkipGrammar<Iterator>>
        guard,
        boolean,
        context_perturb,
        expr
      ;

      ident_parser<Iterator> ident;
    };

    extern template class EquationGrammar<iterator_t>;
  }
}

namespace std
{
  inline ostream&
  operator<<(ostream& os, const TransLucid::Parser::ParsedEquation& e)
  {
    os << "parsed equation";
    return os;
  }
}

#endif // EQUATION_PARSER_HPP_INCLUDED
