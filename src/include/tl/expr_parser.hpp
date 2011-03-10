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

#include <tl/ast.hpp>
#include <tl/builtin_types.hpp>
#include <tl/parser_fwd.hpp>
#include <tl/parser_header_util.hpp>
#include <tl/parser_util.hpp>
#include <tl/utility.hpp>

#include <unordered_map>

#include <boost/spirit/include/qi_auxiliary.hpp>
#include <boost/spirit/include/qi_nonterminal.hpp>
#include <boost/spirit/include/qi_sequence.hpp>
#include <boost/spirit/include/qi_action.hpp>
#include <boost/spirit/include/qi_core.hpp>

#include <boost/spirit/home/phoenix/object/new.hpp>
#include <boost/spirit/home/phoenix/object/construct.hpp>
#include <boost/spirit/home/phoenix/container.hpp>
#include <boost/spirit/home/phoenix/bind/bind_function.hpp>
#include <boost/spirit/home/phoenix/statement/sequence.hpp>

namespace TransLucid
{
  namespace Parser
  {
    namespace ph = boost::phoenix;
    using namespace ph;

    inline char_type
    get_end_char(const Delimiter& d)
    {
      return d.end;
    }

    inline u32string
    get_name(const Delimiter& d)
    {
      return d.type;
    }

    inline Tree::Expr
    construct_typed_constant(const u32string& type, const u32string& value)
    {
      if (type == U"ustring") {
        if (!validate_ustring(value)) {
          throw ParseError(U"Invalid character in ustring");
        }
        return value;
      } else if (type == U"uchar") {
        char32_t v = value[0];
        if (!validate_uchar(v)) {
          throw ParseError(U"Invalid character");
        }
        return v;
      } else {
        return Tree::ConstantExpr(type, value);
      }
    }

    inline u32string
    construct_arg_delim(int a)
    {
      std::ostringstream os;
      os << "arg" << a;
      const std::string& s = os.str();
      return std::u32string(s.begin(), s.end());
    }

    inline Tree::Expr
    construct_delimited_constant(Delimiter& d, const u32string& v)
    {
      return construct_typed_constant(d.type, v);
    }

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
      ExprGrammar(Header& h, TokenDef& tok)
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
        range_expr,
        integer,
        prefix_expr,
        hash_expr,
        context_perturb,
        paren_expr,
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

    extern template class ExprGrammar<iterator_t>;
    extern template ExprGrammar<iterator_t>::ExprGrammar<Lexer::tl_lexer>
      (Header& h, Lexer::tl_lexer& tok);
  }
}

#endif // EXPR_PARSER_HPP_INCLUDED
