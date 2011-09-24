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
    //construct_typed_constant(const u32string& type, const u32string& value)

    /**
     * Construct a constant with a type.
     * When the type is ustring or uchar, it constructs the actual ustring
     * or uchar. Otherwise it constructs a Tree::LiteralExpr.
     */
    construct_typed_constant(const std::pair<u32string, u32string>& c)
    {
      const std::u32string& type = c.first;
      const std::u32string& value = c.second;
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
        return Tree::LiteralExpr(type, value);
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

    struct pair_second_imp
    {
      template <typename Arg>
      struct result
      {
        typedef typename Arg::second_type& type;
      };

      template <typename Arg>
      typename Arg::second_type&
      operator()(Arg& p) const
      {
        return p.second;
      }
    };

    ph::function<pair_second_imp> pair_second;

    struct pair_first_imp
    {
      template <typename Arg>
      struct result
      {
        typedef typename Arg::first_type& type;
      };

      template <typename Arg>
      typename Arg::first_type&
      operator()(Arg& p) const
      {
        return p.first;
      }
    };

    ph::function<pair_first_imp> pair_first;

    /**
     * Create an arg dimension name.
     * @param a The number to append to arg.
     * @return "arg"a.
     */
    inline u32string
    construct_arg_delim(int a)
    {
      std::ostringstream os;
      os << "arg" << a;
      const std::string& s = os.str();
      return std::u32string(s.begin(), s.end());
    }

    inline Tree::UnaryOperator
    find_unary_operator
    (
      const u32string& symbol,
      const System::IdentifierLookup& idents,
      dimension_index symbolDim,
      Context*& k,
      Tree::UnaryType type
    )
    {
      //lookup ATL_SYMBOL

      ContextPerturber p(*k, {{symbolDim, Types::String::create(symbol)}});

      WS* atlWS = idents.lookup(U"ATL_SYMBOL");
      Constant atl = (*atlWS)(*k);

      return Tree::UnaryOperator
        {get_constant_pointer<u32string>(atl), symbol, type};
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
    find_binary_operator
    (
      const u32string& symbol, 
      const System::IdentifierLookup& idents,
      dimension_index symbolDim,
      Context*& k
    )
    {
      //lookup ATL_SYMBOL, ASSOC, PREC

      //std::cerr << "looking up symbol " << symbol << std::endl;

      ContextPerturber p(*k, {{symbolDim, Types::String::create(symbol)}});
      WS* atlWS = idents.lookup(U"ATL_SYMBOL");
      Constant atl = (*atlWS)(*k);

      WS* assocWS = idents.lookup(U"ASSOC");
      Constant assoc = (*assocWS)(*k);

      WS* precWS = idents.lookup(U"PREC");
      Constant prec = (*precWS)(*k);

      const u32string& assocName = get_constant_pointer<u32string>(assoc);

      Tree::InfixAssoc ia = Tree::ASSOC_LEFT;
      if (assocName == U"LEFT")
      {
        ia = Tree::ASSOC_LEFT;
      }
      else if (assocName == U"RIGHT")
      {
        ia = Tree::ASSOC_RIGHT;
      }
      else if (assocName == U"NON")
      {
        ia = Tree::ASSOC_NON;
      }

      #if 0
      std::cerr << "retrieved op" << std::endl
                << "  symbol: " << symbol << std::endl
                << "  op    : " << get_constant_pointer<u32string>(atl)
                << std::endl
                << "  assoc : " << assocName << std::endl
                << "  prec  : " << get_constant_pointer<mpz_class>(prec)
                << std::endl;
      #endif

      return Tree::BinaryOperator
      {
        ia,
        get_constant_pointer<u32string>(atl),
        symbol,
        get_constant_pointer<mpz_class>(prec)
      };
    }

    Tree::Expr
    make_where_clause
    (
      const Tree::Expr& e,
      const WhereSymbols& defs
    )
    {
      Tree::WhereExpr w;

      w.e = e;
      w.dims = defs.first;
      w.vars = defs.second;

      return w;
    }

    void
    append_dim
    (
      std::vector<std::pair<u32string, Tree::Expr>>& dims,
      const u32string& name,
      const Tree::Expr& expr
    )
    {
      dims.push_back(std::make_pair(name, expr));
    }

    void 
    append_eqn
    (
      std::vector<Equation>& eqns,
      std::pair<Equation, DeclType>& e
    )
    {
      eqns.push_back(e.first);
    }

    /**
     * Construct an expression grammar.
     * @param h The parser header to use.
     */
    template <typename Iterator>
    template <typename TokenDef>
    ExprGrammar<Iterator>::ExprGrammar(TokenDef& tok, System& system)
    : ExprGrammar::base_type(expr)
    , m_idents(system.lookupIdentifiers())
    , m_dimName(system.getDimensionIndex(U"name"))
    , m_symbolDim(system.getDimensionIndex(U"symbol"))
    {
      using namespace qi::labels;

      expr %= where_expr
      ;

      where_expr = if_expr[_a = _1] >> 
      (
        (  
           tok.where_
        >> where_inside
        >> tok.end_
        ) 
        [_val = ph::bind(&make_where_clause, _a, _2)]
      | qi::eps[_val = _a]
      )
      ;

      where_inside =
      *(
         (
           (tok.dimension_ >> tok.identifier_ >> tok.maps_ >> expr)
           [
             ph::bind(
               &append_dim, 
               pair_first(_val), 
               _2, 
               _4)
           ]
         |
           (tok.var_ >> eqn)
           [
            ph::bind(&append_eqn, pair_second(_val), _2)
           ]
         ) >> tok.dblsemi_
       )
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
          _val = construct<Tree::IfExpr>(
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
           *(   tok.binary_op_
             >> prefix_expr
            )
            [
              _a = ph::bind(&Tree::insert_binary_operator, 
                     ph::bind(&find_binary_operator, 
                       _1,
                       ph::ref(m_idents),
                       m_symbolDim,
                       ph::ref(m_context)), 
                     _a, _2)
            ]
         )
         [
           _val = _a
         ]
      ;

      prefix_expr =
        ( tok.prefix_op_ > postfix_expr)
        [
          _val = construct<Tree::UnaryOpExpr>
                 (  
                   ph::bind(&find_unary_operator,
                            _1, 
                            ph::ref(m_idents),
                            m_symbolDim,
                            ph::ref(m_context),
                            Tree::UNARY_PREFIX
                           ),
                   _2
                 )
        ]
      | postfix_expr [_val = _1]
      ;

      postfix_expr =
         at_expr [_a = _1]
      >> (
           ( tok.postfix_op_
             [
               _val = construct<Tree::UnaryOpExpr>
                      (
                        ph::bind(&find_unary_operator,
                                 _1, 
                                 ph::ref(m_idents),
                                 m_symbolDim,
                                 ph::ref(m_context),
                                 Tree::UNARY_POSTFIX
                                ),
                        _a
                      )
             ]
           )
         | qi::eps
           [
            _val = _a
           ]
         )
        //at_expr[_val = _1]
      ;

      at_expr =
         hash_expr [_a = _1]
      >> (
           (tok.at_ >> at_expr)
             [
               _val = construct<Tree::AtExpr>(_a, _2)
             ]
         | qi::eps [_val = _a]
         )
      ;

      hash_expr =
        ( tok.hash_ > hash_expr [_val = construct<Tree::HashExpr>(_1)])
        | phi_application [_val = _1]
      ;

      phi_application =
        ( 
         lambda_application[_a = _1]
      >> *(
            lambda_application[_a = construct<Tree::PhiAppExpr>(_a, _1)]
          )
        )
        [
          _val = _a
        ]
      ;
      //phi_application %= lambda_application;

      lambda_application =
        (
         primary_expr[_a = _1] 
      >> *(
            tok.dot_ > primary_expr
            [
              _a = construct<Tree::LambdaAppExpr>(_a, _1)
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
      | context_perturb(false) [_val = _1]
      | (tok.lparen_ > expr > tok.rparen_) 
        [
          _val = construct<Tree::ParenExpr>(_2)
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
            ph::ref(m_idents),
            ph::ref(m_context),
            m_dimName
          )
        ]
      | tok.constantINTERPRET_
        [
          _val = ph::bind(construct_typed_constant, _1)
        ]
      | tok.constantRAW_
        [
          _val = ph::bind(construct_typed_constant, _1)
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
            _val = construct<Tree::PhiExpr>(_2, _4)
          ]
          //lambda abstraction
        | (tok.slash_ > tok.identifier_ > tok.arrow_ > expr)
          [
            _val = construct<Tree::LambdaExpr>(_2, _4)
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
        << val("Error! Expecting '")
        << _4
        //<< val(" here: \"")
        //<< construct<std::string>(_3, _2)
        << val("'")
        << std::endl
      );

    }
  
    ExprGrammar<iterator_t>*
    create_expr_grammar(Lexer::tl_lexer& l, System& system)
    {
      return new ExprGrammar<iterator_t>(l, system);
    }
  }
}
