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

/** @file expr_parser.cpp
 * The expression parser implementation.
 */

#include <tl/ast.hpp>
#include <tl/builtin_types.hpp>
#include <tl/expr_parser.hpp>
#include <tl/parser_header.hpp>
#include <tl/parser_util.hpp>
#include <tl/utility.hpp>

#include <unordered_map>

#include <boost/spirit/include/qi_auxiliary.hpp>
#include <boost/spirit/include/qi_nonterminal.hpp>
#include <boost/spirit/include/qi_sequence.hpp>
#include <boost/spirit/include/qi_action.hpp>
#include <boost/spirit/include/qi_core.hpp>
#include <boost/fusion/include/vector.hpp>

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

    /**
     * Create the elsif list. Creates a vector of pairs as an elsif list from
     * a fusion vector which spirit gives us.
     */
    struct make_elsifs_imp
    {
      /**
       * Metafunction for the return type of make_elsifs_imp.
       */
      template <typename Arg>
      struct result
      {
        /**
         * The actual return type.
         */
        typedef std::vector<std::pair<Tree::Expr, Tree::Expr>> type;
      };

      /**
       * The actual function which does the conversion.
       * @param eif The elsif list.
       */
      template <typename Arg>
      std::vector<std::pair<Tree::Expr, Tree::Expr>>
      operator()(const Arg& eif) const
      {
        using boost::fusion::at_c;

        std::vector<std::pair<Tree::Expr, Tree::Expr>> result;
        for (auto& v : eif)
        {
          result.push_back(std::make_pair(at_c<0>(v), at_c<1>(v)));
        }

        return result;
      }
    };

    /** The phoenix function for creating an elsif list. */
    ph::function<make_elsifs_imp> make_elsifs;

    inline u32string
    construct_arg_delim(int a)
    {
      std::ostringstream os;
      os << "arg" << a;
      const std::string& s = os.str();
      return std::u32string(s.begin(), s.end());
    }

    /**
     * Finds a binary operator from a string.
     *
     * Tries to match the string @a s with an existing binary operator.
     * If it doesn't exist, then it uses the default error operator and
     * flags the parser that there was a problem.
     * @param s The text of the operator.
     * @param h The parser header.
     * @return The binary operator instance.
     * @todo Implement the error flagging.
     */
    inline Tree::BinaryOperator
    find_binary_operator(const u32string& s, const Header& h)
    {
      auto iter = h.binary_op_symbols.find(s);
      if (iter != h.binary_op_symbols.end())
      {
        return iter->second;
      }
      else
      {
        throw "invalid binary operator: " + utf32_to_utf8(s);
      }
    }

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
          qi::_val = construct<Tree::IfExpr>(
            _2, _4, make_elsifs(_5), _7)
        ]
      | binary_op
        [
          _val = _1
        ]
      ;

      binary_op =
         prefix_expr [_a = _1]
      >> (
           *(   tok.any_
             >> prefix_expr
            )
            [
              _a = ph::bind(&Tree::insert_binary_operator, 
                     ph::bind(&find_binary_operator, 
                       //ph::construct<u32string>(ph::begin(_1), ph::end(_1)), 
                       _1,
                       ph::ref(header)), 
                     _a, _2)
            ]
         )
         [
           _val = _a
         ]
      ;

      prefix_expr =
      #if 0
        ( header.prefix_op_symbols >> postfix_expr)
        [
          _val = construct<Tree::UnaryOpExpr>(_1, _2)
        ]
      | 
      #endif 
      postfix_expr [_val = _1]
      ;

      postfix_expr =
      #if 0
         at_expr [_a = _1]
      >> (
           ( tok.any_
             [
               _val = construct<Tree::UnaryOpExpr>(_1, _a)
             ]
           )
         | qi::eps
           [
            _val = _a
           ]
         )
      #endif
        at_expr[_val = _1]
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

      primary_expr =
        tok.integer_[_val = construct<mpz_class>(_1)]
      | boolean [_val = _1]
      //| dimensions
      //| specials
      | ident_constant [_val = _1]
      | context_perturb [_val = _1]
      | (literal('(') >> expr > literal(')')) 
        [
          _val = construct<Tree::ParenExpr>(_1)
        ]
//        | delimiters
      | function_abstraction [_val = _1]
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
  
    ExprGrammar<iterator_t>*
    create_expr_grammar(Header& h, Lexer::tl_lexer& l)
    {
      return new ExprGrammar<iterator_t>(h, l);
    }
  }
}
