/* The expr part of the parser.
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

#ifndef EXPR_PARSER_HPP_INCLUDED
#define EXPR_PARSER_HPP_INCLUDED

#include <tl/parser_fwd.hpp>
#include <tl/parser_header_util.hpp>
#include <tl/parser_util.hpp>

namespace TransLucid
{
  namespace Parser
  {
    template <typename Iterator>
    class ExprGrammar
    : public qi::grammar<Iterator, Tree::Expr()>
    {
      public:

      /**
       * Construct an expression grammar.
       * @param h The parser header to use.
       */
      template <typename TokenDef>
      ExprGrammar(Header& h, TokenDef& tok);

      /**
       * Set the tuple parser for this expression parser.
       * @param t The tuple parser.
       */
      template <typename T>
      void
      set_tuple(const T& t)
      {
        context_perturb = t;
      }

      private:

      qi::rule<Iterator, Tree::Expr()>
        expr,
        if_expr,
        boolean,
        prefix_expr,
        hash_expr,
        context_perturb,
        function_abstraction
      ;

      qi::rule<Iterator, Tree::Expr(),
               qi::locals<Tree::Expr>>
        lambda_application,
        phi_application
      ;

      qi::rule<Iterator, Tree::Expr(), qi::locals<Tree::Expr>>
        postfix_expr,
        at_expr,
        binary_op
      ;

      qi::rule<Iterator, Tree::Expr(), qi::locals<string_type>>
        primary_expr
      ;

      qi::rule<Iterator, Tree::Expr()>
        ident_constant
      ;

      Header &header;
      //this contains a mapping of all the reserved identifiers to
      //their expression tree
      //the place that this is stored might change in the future
      ReservedIdentifierMap m_reserved_identifiers;
    };
 
    ExprGrammar<iterator_t>*
    create_expr_grammar(Header&, Lexer::tl_lexer&);
  }
}

#endif // EXPR_PARSER_HPP_INCLUDED
