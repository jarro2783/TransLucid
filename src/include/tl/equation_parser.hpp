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
#include <tl/parser_api.hpp>
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
    : public qi::grammar
      <
        Iterator, 
        Equation(),
        qi::locals<u32string>
      >
    {
      public:

      /**
       * Construct an Equation Grammar.
       * @param tok The lexer from which to match tokens.
       */
      template <typename TokenDef>
      EquationGrammar(TokenDef& tok)
      : EquationGrammar::base_type(equation)
      {
        using namespace qi::labels;

        equation =
          (
              tok.identifier_[_a = _1]
           >> guard
           >> boolean
           >> tok.def_
           >> expr
          )
          [
            _val = construct<Equation>(_a, _2, _3, _4)
          ]
           >  tok.dblsemi_
        ;

        guard =
          ( tok.pipe_ >> context_perturb[_val = _1])
        | qi::eps [_val = construct<Tree::nil>()]
        ;

        boolean =
          ( tok.and_ >> expr[_val = _1])
        | qi::eps [_val = construct<Tree::nil>()]
        ;

        BOOST_SPIRIT_DEBUG_NODE(boolean);
        BOOST_SPIRIT_DEBUG_NODE(guard);
        BOOST_SPIRIT_DEBUG_NODE(equation);
        BOOST_SPIRIT_DEBUG_NODE(expr);
        BOOST_SPIRIT_DEBUG_NODE(context_perturb);
      }

      /**
       * Set the tuple parser for this equation parser.
       * @param t The tuple parser.
       */
      template <typename T>
      void
      set_tuple(const T& t)
      {
        using namespace qi::labels;
        context_perturb = t[_val = _1];
      }

      /**
       * Set the expression parser for this equation parser.
       * @param t The expression parser.
       */
      template <typename T>
      void
      set_expr(const T& t)
      {
        expr %= t;
      }

      private:

      qi::rule<Iterator, Equation(), qi::locals<u32string>>
        equation
      ;

      qi::rule<Iterator, Tree::Expr()>
        guard,
        boolean,
        context_perturb,
        expr
      ;
    };

    extern template class EquationGrammar<iterator_t>;
  }
}

namespace std
{
  inline ostream&
  operator<<(ostream& os, const TransLucid::Parser::Equation& e)
  {
    os << "parsed equation";
    return os;
  }
}

#endif // EQUATION_PARSER_HPP_INCLUDED
