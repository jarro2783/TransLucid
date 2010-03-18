/* Header and Equation Parser.
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

//TODO: refactor this into two files

#ifndef TL_PARSER_INCLUDED
#define TL_PARSER_INCLUDED

#include <tl/parser_util.hpp>
#include <tl/parser_fwd.hpp>
#include <boost/spirit/include/qi_auxiliary.hpp>
#include <boost/spirit/home/phoenix/object/construct.hpp>

namespace TransLucid
{
  namespace Parser
  {
    using namespace boost::phoenix;
    namespace ph = boost::phoenix;

    struct add_dimension_impl
    {
      template <typename Arg1, typename Arg2>
      struct result
      {
        typedef void type;
      };

      template <typename Arg1, typename Arg2>
      void
      operator()(Arg1 arg1, Arg2 arg2) const
      {
        arg1.dimension_symbols.add
          (arg2.c_str(), std::u32string(arg2.begin(), arg2.end()));
      }
    };

    namespace
    {
      function<add_dimension_impl> add_dimension;
    }

    #if 0
    inline void
    addOpDefinition
    (
      Header& header,
      InfixAssoc assoc,
      const std::u32string& op,
      const std::u32string& symbol,
      AST::Expr* precedence
    )
    {
      size_t pos = header.binary_op_info.size();
      if (symbol.size() == 1)
      {
         header.binary_op_symbols.add(symbol.c_str(), pos);
      }
      std::u32string underscoreSymbol = U"_" + symbol + U"_";
      header.binary_op_symbols.add(underscoreSymbol.c_str(), pos);
      AST::IntegerExpr* p = dynamic_cast<AST::IntegerExpr*>(precedence);
      header.binary_op_info.push_back
        (BinaryOperation(assoc, op, symbol, p->m_value));
    }
    #endif

    inline void
    addOpSymbol
    (
      Header& h,
      const string_type& symbol,
      const string_type& opName,
      Tree::InfixAssoc assoc,
      const mpz_class& precedence
    )
    {
      h.binary_op_symbols.add
      (
        symbol.c_str(),
        Tree::BinaryOperation
        (
          assoc,
          to_u32string(opName),
          to_u32string(symbol),
          precedence
        )
      );
    }

    inline void
    addDelimiter
    (
      Header& header,
      const Tree::Expr& type,
      const Tree::Expr& open,
      const Tree::Expr& close
      //const u32string& type,
      //const string_type& open,
      //const string_type& close
    )
    {
      try
      {
        const u32string& ctype =
          boost::get<u32string&>(type);
        const char32_t& copen =
          boost::get<const char32_t&>(open);
        const char32_t& cclose =
          boost::get<const char32_t&>(cclose);

        string_type open_string(1, copen);
        header.delimiter_start_symbols.add
        (
          open_string.c_str(),
          Delimiter(ctype, (char_type)copen, (char_type)cclose)
        );
      }
      catch (const boost::bad_get&)
      {
      }
      catch (const std::invalid_argument&)
      {
      }
      //std::cout << "adding " << open << " " << close << std::endl;
      //if
      //(
         //header.delimiter_start_symbols.add
         //(
         //  open.c_str(),
         //  Delimiter(type, open[0], close[0])
         //);
      //)
      //{
      //  throw ParseError("open delimiter '" + ustring_t(open)
      //                                      + "' already defined");
      //}
      //else
      //{
      //}
    }

    template <typename Iterator>
    class HeaderGrammar :
      public qi::grammar<Iterator, skip, Header()>
    {
      public:

      HeaderGrammar()
      : HeaderGrammar::base_type(headerp)
      {
         using namespace qi::labels;

         assoc_symbols.add
           (L"infixl", Tree::ASSOC_LEFT)
           (L"infixr", Tree::ASSOC_RIGHT)
           (L"infixn", Tree::ASSOC_NON)
           (L"infixp", Tree::ASSOC_COMPARISON)
           (L"infixm", Tree::ASSOC_VARIABLE)
         ;

         headerp =
           *( headerItem(_val) >> qi::lit( ";;" ))
            >> qi::eoi;
         ;

         headerItem =
           (
             qi::lit("dimension")
               > angle_string
                 [
                    add_dimension(_r1, _1)
                 ]
           )
         | (
               assoc_symbols
            >> "ustring"
            >> angle_string
            >> "ustring"
            >> angle_string
            >> integer
           )

         | (
               qi::string("delimiters")
            >  expr
            >  expr
            >  expr
           )
           [
             ph::bind(&addDelimiter, _r1, _1, _2, _3)
           ]
         | (
               qi::string("library")
            >> "ustring"
            >> angle_string
           )
         |    (
                qi::string("prefix")
              | "postfix"
              )
           >> "ustring"
           >> angle_string
           >> "ustring"
           >> angle_string
         ;

         integer = qi::int_;

         //constant = self.parsers.constant_parser.top();

         //BOOST_SPIRIT_DEBUG_RULE(constant);
      }

      template <class T>
      void
      set_expr(const T& e)
      {
        using namespace qi::labels;
        expr = e[_val = _1];
      }

      private:

      qi::rule<Iterator, skip, Header()>
        headerp
      ;

      qi::rule<Iterator, skip, void(Header&)>
        headerItem
      ;

      qi::rule<Iterator>
        integer
      ;

      qi::rule<Iterator>
        expr
      ;

      //qi::rule<Iterator, string_type()> angle_string;

      escaped_string_parser<Iterator> angle_string;

      qi::symbols<char_type, Tree::InfixAssoc> assoc_symbols;
      Tree::InfixAssoc currentAssoc;

      //Spirit::assertion<ParseErrorType> expect_dbl_semi;

      //AngleStringGrammar angle_string;
    };

    //name, | [], & bool, = HD
    typedef std::tuple<string_type, Tree::Expr, Tree::Expr, Tree::Expr>
    ParsedEquation;

    template <typename Iterator>
    class EquationGrammar
    : public qi::grammar<Iterator, ParsedEquation(),qi::locals<string_type>,
      SkipGrammar<Iterator>>
    {
      public:

      EquationGrammar()
      : EquationGrammar::base_type(equation)
      {
        using namespace qi::labels;

        equation =
          (
              ident[_a = _1]
           >> guard
           >> boolean
           >> L'='
           >> expr
          )
          [
            _val = construct<ParsedEquation>(_a, _2, _3, _4)
          ]
        ;

        guard =
          ( qi::lit('|') >> context_perturb[_val = _1])
        | qi::eps [_val = construct<Tree::nil>()]
        ;

        boolean =
          ( qi::lit('&') >> expr[_val = _1])
        | qi::eps [_val = construct<Tree::nil>()]
        ;

        BOOST_SPIRIT_DEBUG_NODE(boolean);
        BOOST_SPIRIT_DEBUG_NODE(guard);
        BOOST_SPIRIT_DEBUG_NODE(equation);
        BOOST_SPIRIT_DEBUG_NODE(expr);
        BOOST_SPIRIT_DEBUG_NODE(context_perturb);
      }

      template <typename T>
      void
      set_context_perturb(const T& t)
      {
        using namespace qi::labels;
        context_perturb = t[_val = _1];
      }

      template <typename T>
      void
      set_expr(const T& t)
      {
        expr %= t;
      }

      private:

      qi::rule<Iterator, ParsedEquation(), qi::locals<string_type>,
               SkipGrammar<Iterator>>
        equation
      ;

      qi::rule<Iterator, Tree::Expr(), SkipGrammar<Iterator>>
        guard,
        boolean,
        context_perturb,
        expr
      ;

      ident_parser<Iterator> ident;
    };
  }
}

#endif // TL_PARSER_INCLUDED
