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

/**
 * @file expr_parser.hpp
 * The expression parser header.
 */

#include <tl/parser_header.hpp>
#include <tl/parser_util.hpp>

namespace TransLucid
{
  namespace Parser
  {
    typedef std::pair
    <
      std::vector<std::pair<u32string, Tree::Expr>>,
      std::vector<Equation>
    > WhereSymbols;

    template <typename Iterator>
    class ExprGrammar
    : public qi::grammar<Iterator, Tree::Expr()>
    {
      public:

      /**
       * Construct an expression grammar.
       * @param h The parser header to use.
       * @param tok The lexer from which to match tokens.
       */
      template <typename TokenDef>
      ExprGrammar(TokenDef& tok, System& system);

      /**
       * Set the tuple parser for this expression parser.
       * @param t The tuple parser.
       */
      template <typename T>
      void
      set_tuple(const T& t)
      {
        using namespace qi::labels;
        context_perturb = t(_r1);
      }

      template <typename T>
      void
      set_equation(const T& e)
      {
        eqn = e;
      }

      Context* m_context;

      private:

      qi::rule<Iterator, Tree::Expr()>
        expr,
        if_expr,
        boolean,
        prefix_expr,
        hash_expr,
        function_abstraction
      ;

      qi::rule<Iterator, Tree::Expr(bool)>
        context_perturb
      ;

      qi::rule
      <
        Iterator, 
        std::pair<Equation, DeclType>()
      > eqn
      ;

      qi::rule
      <
        Iterator, 
        Tree::Expr(),
        qi::locals<Tree::Expr>
      >
        binary_op,
        postfix_expr,
        where_expr,
        app_expr
      ;

      qi::rule
      <
        Iterator,
        Tree::Expr(Tree::Expr)
      >
        token_app
      ;

      qi::rule
      <
        Iterator,
        std::vector<Tree::Expr>()
      >
        expr_list
      ;

      qi::rule<Iterator, Tree::Expr(), qi::locals<string_type>>
        primary_expr
      ;

      qi::rule<Iterator, Tree::Expr()>
        ident_constant
      ;

      qi::rule<Iterator, WhereSymbols()>
        where_inside
      ;

      System::IdentifierLookup m_idents;
      dimension_index m_dimName;
      dimension_index m_symbolDim;
    };
 
    /**
     * Creates an ExprGrammar.
     * Allocates an ExprGrammar with new. It should be freed with delete.
     * @param h The header that the parser should use.
     * @param l The lexer that the parser should get symbols from.
     * @return A new ExprGrammar.
     */
    ExprGrammar<iterator_t>*
    create_expr_grammar(Header&, Lexer::tl_lexer&);
  }
}

#endif // EXPR_PARSER_HPP_INCLUDED
