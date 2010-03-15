/* TODO: Give a descriptor.
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

#ifndef CONSTANT_PARSER_HPP_INCLUDED
#define CONSTANT_PARSER_HPP_INCLUDED

#if 0

#include <tl/parser_fwd.hpp>
#include <tl/parser_util.hpp>
#include <tl/expr.hpp>

namespace TransLucid
{
  namespace Parser
  {
    class ConstantGrammar : public Spirit::grammar<ConstantGrammar>
    {
      public:
      ConstantGrammar(Parsers& parsers)
      : parsers(parsers)
      {}

      Parsers& parsers;

      template <typename ScannerT>
      class definition
      {
        public:
        definition(const ConstantGrammar& self)
        : expr_stack(self.parsers.expr_stack),
          string_stack(self.parsers.string_stack),
          angle_string(string_stack)
        {
          constant =
             identifier_p
             [
               push_front(ph::ref(string_stack), arg1)
             ]
          >> angle_string
             [
               push_front(ph::ref(expr_stack),
                 new_<AST::ConstantExpr>(at(ph::ref(string_stack), 1),
                 at(ph::ref(string_stack), 0))),
                 pop_front_n<2>()(ph::ref(string_stack))
             ]
          ;
        }

        private:
        qi::rule<Iterator> constant;

        //std::deque<AST::Expr*>& expr_stack;
        //std::deque<wstring_t>& string_stack;

        AngleStringGrammar angle_string;
      };
    };
  }
}
#endif

#endif // CONSTANT_PARSER_HPP_INCLUDED
