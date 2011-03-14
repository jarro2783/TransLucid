/* Instantiation of ExprGrammar.
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

#include <tl/expr_parser.hpp>

namespace TransLucid
{
  namespace Parser
  {
    /**
     * Construct an expression grammar.
     * @param h The parser header to use.
     */
    template <typename Iterator>
    template <typename TokenDef>
    ExprGrammar<Iterator>::ExprGrammar(Header& h, TokenDef& tok)
    : ExprGrammar::base_type(expr), header(h)
    {
      using namespace qi::labels;

      m_reserved_identifiers = init_reserved_identifiers();

      expr %= if_expr
      ;

      if_expr =
        (
            tok.if_
         >> if_expr
         >> tok.then_
         >> if_expr
         >> *(
                  tok.elsif_
               >> if_expr
               >> tok.then_
               >> if_expr
             )
         >> tok.else_
         >> if_expr
         >> tok.fi_
        )
        [
          qi::_val = construct<Tree::IfExpr>(_2, _4, _5, _7)
        ]
      | range_expr
        [
          _val = _1
        ]
      ;

      range_expr %= binary_op
      ;
       //>> -(".."
       //>> if_expr)
       //;

      //the actions have to be put after the optional with | eps

      binary_op =
         prefix_expr [_a = _1]
      >> (
           *(   header.binary_op_symbols
             >> prefix_expr
            )
            [
              _a = ph::bind(&Tree::insert_binary_operator, _1, _a, _2)
            ]
         )
         [
           _val = _a
         ]
      ;

      prefix_expr =
        ( header.prefix_op_symbols >> postfix_expr)
        [
          _val = construct<Tree::UnaryOpExpr>(_1, _2)
        ]
      | postfix_expr [_val = _1]
      ;

      postfix_expr =
         at_expr [_a = _1]
      >> (
           ( header.postfix_op_symbols
             [
               _val = construct<Tree::UnaryOpExpr>(_1, _a)
             ]
           )
         | qi::eps
           [
            _val = _a
           ]
         )
      ;

      at_expr =
         hash_expr [_a = _1]
      >> (
           (literal('@') >> at_expr)
             [
               _val = construct<Tree::AtExpr>(_a, _1, false)
             ]
         | qi::eps [_val = _a]
         )
      ;

      hash_expr =
        ( literal('#') > hash_expr [_val = construct<Tree::HashExpr>(_1)])
        | phi_application [_val = _1]
      ;

      #if 0
      phi_application =
        ( 
         lambda_application[_a = _1]
      >> *(
            lambda_application[_a = construct<Tree::ValueAppExpr>(_a, _1)]
          )
        )
        [
          _val = _a
        ]
      ;
      #endif
      phi_application %= lambda_application;

      lambda_application =
        (
         primary_expr[_a = _1] 
      >> *(
            literal(".") > primary_expr
            [
              _a = construct<Tree::ValueAppExpr>(_a, _1)
            ]
          )
        )
        [
          _val = _a
        ]
      ;

      primary_expr %=
        integer
      | boolean
      //| dimensions
      //| specials
      | ident_constant
      | context_perturb
      | paren_expr 
//        | delimiters
      | function_abstraction
      ;

      paren_expr =
        (literal('(') >> expr > literal(')')) 
        [
          _val = construct<Tree::ParenExpr>(_1)
        ]
      ;

      ident_constant = 
        tok.identifier_ 
        [
          _val = ph::bind(
            construct_identifier, 
            _1, 
            ph::ref(m_reserved_identifiers)
          )
        ]
      | tok.constantINTERPRET_
        [
          _val = construct<Tree::ConstantExpr>(_1)
        ]
      | tok.constantRAW_
        [
          _val = construct<Tree::ConstantExpr>(_1)
        ]
      | tok.character_[_val = construct<char32_t>(_1)]
      ;
     
      boolean =
        tok.true_
        [
          _val = true
        ]
      | tok.false_
        [
          _val = false
        ]
      ;

      integer = tok.integer_[_val = construct<mpz_class>(_1)];

      function_abstraction = 
          //phi abstraction
          (tok.dblslash_ > tok.identifier_ > tok.arrow_ > expr)
          [
            _val = construct<Tree::PhiExpr>()
          ]
          //lambda abstraction
        | (literal("\\") > tok.identifier_ > tok.arrow_ > expr)
          [
            _val = construct<Tree::LambdaExpr>(_1, _2)
          ]
      ;

      //BOOST_SPIRIT_DEBUG_NODE(if_expr);
      BOOST_SPIRIT_DEBUG_NODE(expr);
      BOOST_SPIRIT_DEBUG_NODE(boolean);
      BOOST_SPIRIT_DEBUG_NODE(range_expr);
      BOOST_SPIRIT_DEBUG_NODE(integer);
      BOOST_SPIRIT_DEBUG_NODE(prefix_expr);
      BOOST_SPIRIT_DEBUG_NODE(hash_expr);
      BOOST_SPIRIT_DEBUG_NODE(context_perturb);
      //BOOST_SPIRIT_DEBUG_NODE(end_delimiter);
      BOOST_SPIRIT_DEBUG_NODE(postfix_expr);
      BOOST_SPIRIT_DEBUG_NODE(at_expr);
      BOOST_SPIRIT_DEBUG_NODE(binary_op);
      BOOST_SPIRIT_DEBUG_NODE(primary_expr);
      BOOST_SPIRIT_DEBUG_NODE(phi_application);
      BOOST_SPIRIT_DEBUG_NODE(lambda_application);

      expr.name("expr");
      integer.name("integer");

      qi::on_error<qi::fail>
      (
        expr,
        std::cerr
        << val("Error! Expecting ")
        << _4
        << val(" here: \"")
        << construct<std::string>(_3, _2)
        << val("\"")
        << std::endl
      );
    }

    template <typename Iterator>
    template <typename T>
    void
    ExprGrammar<Iterator>::set_tuple(const T& t)
    {
      context_perturb = t;
    }

    template class ExprGrammar<iterator_t>;
    template ExprGrammar<iterator_t>::ExprGrammar<Lexer::tl_lexer>
      (Header& h, Lexer::tl_lexer& tok);
  }
}
