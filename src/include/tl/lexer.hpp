/* TransLucid lexer definition.
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

/**
 * @file lexer.hpp
 * The lexer implementation.
 * The variables in the lexer are named such that any variable with an
 * underscore at the end of the name is a token.
 */

#ifndef TL_LEXER_HPP_INCLUDED
#define TL_LEXER_HPP_INCLUDED

#include <boost/spirit/include/lex_lexertl.hpp>
#include <boost/spirit/home/phoenix/operator.hpp>
#include <boost/spirit/home/phoenix/bind.hpp>
#include <boost/spirit/home/phoenix/statement/sequence.hpp>
#include <boost/spirit/home/phoenix/object/construct.hpp>

#include <tl/charset.hpp>
#include <tl/detail/lexer_detail.hpp>
#include <tl/lexer_util.hpp>
#include <tl/parser_api.hpp>
#include <tl/parser_iterator.hpp>
#include <tl/system.hpp>
#include <tl/utility.hpp>

namespace TransLucid
{
  namespace Lexer
  {
    namespace lex = boost::spirit::lex;
    typedef wchar_t lex_char_type;

    // iterator type used to expose the underlying input stream
    typedef std::basic_string<lex_char_type> wstring;
    //typedef wstring::const_iterator base_iterator_type;
    typedef Parser::U32Iterator base_iterator_t;

    // This is the token type to return from the lexer iterator
    typedef lex::lexertl::token<
      base_iterator_t, 
      boost::mpl::vector
      <
        value_wrapper<mpz_class>, 
        value_wrapper<mpq_class>,
        value_wrapper<mpf_class>,
        u32string, 
        std::pair<u32string, u32string>,
        char32_t,
        Tree::UnaryType,
        Tree::InfixAssoc
      > 
    > token_type;

    // This is the lexer type to use to tokenize the input.
    // We use the lexertl based lexer engine.
    typedef lex::lexertl::actor_lexer<token_type> lexer_type;

    //for unnamed tokens
    typedef lex::token_def<lex::unused_type, lex_char_type> token_def_default;

    template <typename Lexer>
    struct lex_tl_tokens : lex::lexer<Lexer>
    {
      lex_tl_tokens(Parser::Errors& errors, System& system);

      lex::token_def<lex::unused_type, lex_char_type> 
        //keywords
        if_, fi_, where_, then_, elsif_, else_, true_, false_
      , library_, dimension_, var_, assignment_
        //symbols
      , arrow_, dblsemi_, dblslash_ //, dbldollar_
      , dblpercent_, assign_
      , maps_
        //white space
      , spaces
      //single characters
      , decl_, lbracket_, rbracket_, dot_, def_, and_, hash_, at_, slash_
      , lparen_, rparen_, pipe_, comma_, dollar_
      ;

      lex::token_def<Tree::InfixAssoc, lex_char_type>
        infix_binary_
      ;

      lex::token_def<Tree::UnaryType, lex_char_type>
        unary_
      ;

      lex::token_def<u32string, lex_char_type>
        identifier_, operator_, range_
        //fake tokens for operators
        , binary_op_, prefix_op_, postfix_op_
      ;

      lex::token_def<value_wrapper<mpz_class>, lex_char_type> 
        integer_
      ;

      lex::token_def<value_wrapper<mpf_class>, lex_char_type>
        real_
      ;

      lex::token_def<value_wrapper<mpq_class>, lex_char_type>
        rational_
      ;

      lex::token_def<std::pair<u32string, u32string>, lex_char_type>
      //these are the two that the parser should match
        constantINTERPRET_
      , constantRAW_
      ;

      lex::token_def<char32_t, lex_char_type> 
        character_
      ;

      //this must be set beforing entering the lexer, unfortunately
      //there is no way to ensure this, so someone will probably forget...
      const Tuple* m_context;

      private:
      //std::wstring m_constant_type;
      //std::wstring m_constant_value;
      Parser::Errors& m_errors;
      System::IdentifierLookup m_identifiers;
      dimension_index m_symbolDim;
    };

    //the lexer class
    typedef lex_tl_tokens<lexer_type> tl_lexer;

    // This is the iterator type exposed by the lexer 
    typedef tl_lexer::iterator_type iterator_t;
  }
}

#endif
