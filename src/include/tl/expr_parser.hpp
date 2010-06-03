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

#include <tl/parser_util.hpp>
#include <tl/parser_fwd.hpp>
#include <tl/utility.hpp>
#include <tl/builtin_types.hpp>
#include <boost/spirit/include/qi_auxiliary.hpp>

#include <boost/spirit/home/phoenix/object/new.hpp>
#include <boost/spirit/home/phoenix/object/construct.hpp>
#include <boost/spirit/home/phoenix/container.hpp>
#include <boost/spirit/home/phoenix/bind/bind_function.hpp>
#include <boost/spirit/home/phoenix/statement/sequence.hpp>

#include <tl/ast.hpp>

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

    inline Tree::Expr
    construct_delimited_constant(Delimiter& d, const u32string& v)
    {
      return construct_typed_constant(d.type, v);
    }

    template <typename Iterator>
    class ExprGrammar
    : public qi::grammar<Iterator, Tree::Expr(), SkipGrammar<Iterator>>
    {
      public:

      ExprGrammar(Header& h)
      : ExprGrammar::base_type(expr), header(h)
      {
        //using qi::_val;
        using namespace qi::labels;

        specials.add
          (L"sperror", Special::ERROR)
          (L"spaccess", Special::ACCESS)
          (L"sptype", Special::TYPEERROR)
          (L"spdim", Special::DIMENSION)
          (L"spundef", Special::UNDEF)
          (L"spconst", Special::CONST)
          (L"sploop", Special::LOOP)
          (L"spmultidef", Special::MULTIDEF)
        ;
        for 
        (
          auto iter = Special::m_sv.stov.begin(); 
          iter != Special::m_sv.stov.end();
          ++iter
        )
        {
          specials.add(
            Parser::string_type(iter->first.begin(), iter->first.end()), 
            iter->second);
        }

        expr %= if_expr
        ;

        if_expr =
          (
              qi::lit("if")
           >> if_expr
           >> qi::lit("then")
           >> if_expr
           >> *(
                    "elsif"
                 >> if_expr
                 >> qi::lit("then")
                 >> if_expr
               )
           >> qi::lit("else")
           >> if_expr
           >> qi::lit("fi")
          )
          [
            qi::_val = construct<Tree::IfExpr>(_1, _2, _3, _4)
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
               ("@@" >> at_expr)
               [
                 _val = construct<Tree::AtExpr>(_a, _1, true)
               ]
             |
               ('@' >> at_expr)
               [
                 _val = construct<Tree::AtExpr>(_a, _1, false)
               ]
           | qi::eps [_val = _a]
           )
        ;

        hash_expr =
          ( '#' > hash_expr [_val = construct<Tree::HashExpr>(_1)])
          | primary_expr [_val = _1]
        ;

        primary_expr =
          integer [_val = _1]
        | boolean [_val = _1]
        | header.dimension_symbols
          [
            _val = construct<Tree::DimensionExpr>(make_u32string(_1))
          ]
        | specials [_val = _1]
        | ident [_a = _1]
          >> ( angle_string
               [
                 //_val = construct<Tree::ConstantExpr>(make_u32string(_a),
                 //                                     make_u32string(_1))
                 _val = ph::bind
                 (
                   &construct_typed_constant, 
                   make_u32string(_a), 
                   make_u32string(_1)
                 )
               ]
             | qi::eps
               [
                _val = construct<Tree::IdentExpr>(make_u32string(_a))
               ]
             )
        | context_perturb [_val = _1]
        | ('(' >> expr >> ')') [_val = _1]
        | delimiters [_val = _1]
        ;

        delimiters = 
          (   header.delimiter_start_symbols
              [
                _b = _1,
                _a = construct<string_type>()
              ]
           >> (
               qi::lexeme
               [
                 *(qi::standard_wide::char_ -
                   end_delimiter(ph::bind(&get_end_char, _b)))
                  [
                    _a += _1
                  ]
               ]
              )
            > end_delimiter(ph::bind(&get_end_char, _b))
          )
          [
            _val = ph::bind(&construct_delimited_constant, _b,
                            make_u32string(_a))
          ]
        ;

        boolean =
          qi::ascii::string("true")
          [
            _val = true
          ]
        | qi::ascii::string("false")
          [
            _val = false
          ]
        ;

        integer = integer_grammar[_val = _1];

        end_delimiter = qi::standard_wide::char_(_r1);

        BOOST_SPIRIT_DEBUG_NODE(if_expr);
        BOOST_SPIRIT_DEBUG_NODE(expr);
        BOOST_SPIRIT_DEBUG_NODE(boolean);
        BOOST_SPIRIT_DEBUG_NODE(range_expr);
        BOOST_SPIRIT_DEBUG_NODE(integer);
        BOOST_SPIRIT_DEBUG_NODE(prefix_expr);
        BOOST_SPIRIT_DEBUG_NODE(hash_expr);
        BOOST_SPIRIT_DEBUG_NODE(context_perturb);
        BOOST_SPIRIT_DEBUG_NODE(end_delimiter);
        BOOST_SPIRIT_DEBUG_NODE(postfix_expr);
        BOOST_SPIRIT_DEBUG_NODE(at_expr);
        BOOST_SPIRIT_DEBUG_NODE(binary_op);
        BOOST_SPIRIT_DEBUG_NODE(primary_expr);
        //BOOST_SPIRIT_DEBUG_NODE(integer_grammar);

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

      template <typename T>
      void
      set_tuple(const T& t)
      {
        context_perturb = t;
      }

      private:

      char_type end;

      qi::rule<Iterator, Tree::Expr(), SkipGrammar<Iterator>>
        expr,
        if_expr,
        boolean,
        range_expr,
        integer,
        prefix_expr,
        hash_expr,
        context_perturb
      ;

      qi::rule<Iterator, void(char_type)>
        end_delimiter
      ;

      qi::rule<Iterator, Tree::Expr(), qi::locals<Tree::Expr>,
               SkipGrammar<Iterator>>
        postfix_expr,
        at_expr,
        binary_op
      ;

      qi::rule<Iterator, Tree::Expr(), qi::locals<string_type, Delimiter>,
               SkipGrammar<Iterator>>
        primary_expr,
        delimiters
      ;

      integer_parser<Iterator> integer_grammar;
      escaped_string_parser<Iterator> angle_string;
      ident_parser<Iterator> ident;

      Header &header;

      qi::symbols<char_type, Special::Value> specials;

      #warning do error checking properly
    };

    extern template class ExprGrammar<string_type::const_iterator>;
  }
}

#endif // EXPR_PARSER_HPP_INCLUDED
