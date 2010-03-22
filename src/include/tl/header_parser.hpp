/* Header Parser.
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

#ifndef TL_PARSER_INCLUDED
#define TL_PARSER_INCLUDED

#include <tl/parser_util.hpp>
#include <tl/parser_fwd.hpp>
#include <boost/spirit/include/qi_auxiliary.hpp>
#include <boost/spirit/home/phoenix/object/construct.hpp>

#warning check for duplicate added symbols

namespace TransLucid
{
  namespace Parser
  {
    namespace ph = boost::phoenix;
    using namespace ph;

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
    addDimensionSymbol(Header& h, const u32string& name)
    {
      string_type wsname(name.begin(), name.end());
      h.dimension_symbols.add(wsname.c_str(), name);
    }

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
    addDelimiterSymbol
    (
      Header& header,
      const u32string& type,
      char32_t open,
      char32_t close
    )
    {
      string_type open_string(1, open);
      header.delimiter_start_symbols.add
      (
        open_string.c_str(),
        Delimiter(type, (char_type)open, (char_type)close)
      );
    }

    inline void
    addUnaryOpSymbol
    (
      Header& header,
      Tree::UnaryType type,
      const string_type& symbol,
      const string_type& op
    )
    {
      Tree::UnaryOperation opinfo
      (
        to_u32string(op),
        to_u32string(symbol),
        type
      );

      if (type == Tree::UNARY_PREFIX)
      {
        header.prefix_op_symbols.add
        (
          symbol.c_str(),
          opinfo
        );
      }
      else
      {
        header.postfix_op_symbols.add
        (
          symbol.c_str(),
          opinfo
        );
      }
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
               > expr
                 [
                    //add_dimension(_r1, _1)
                    ph::bind(&addDimensionSymbol, _r1, _1)
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
            >> expr
           )
         | (
              (
                qi::string("prefix")
              | "postfix"
              )
           >> expr
           >> expr
           )
           [
             ph::bind(&HeaderGrammar<Iterator>::addUnary, _r1, _1, _2, _3)
           ]
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

      static void
      addUnary
      (
        Header& header,
        const string_type& type,
        const Tree::Expr& symbol,
        const Tree::Expr& op
      )
      {
        try
        {
          const u32string& csymbol =
            boost::get<const u32string&>(symbol);
          const u32string& coperation =
            boost::get<const u32string&>(op);

          Tree::UnaryType actual_type;
          if (type == L"prefix")
          {
            actual_type = Tree::UNARY_PREFIX;
          }
          else
          {
            actual_type = Tree::UNARY_POSTFIX;
          }

          addUnaryOpSymbol
          (
            header,
            actual_type,
            string_type(csymbol.begin(), csymbol.end()),
            string_type(coperation.begin(), coperation.end())
          );
        }
        catch (const boost::bad_get&)
        {
        }
      }

      static void
      addDelimiter
      (
        Header& header,
        const Tree::Expr& type,
        const Tree::Expr& open,
        const Tree::Expr& close
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

          addDelimiterSymbol(header, ctype, copen, cclose);
        }
        catch (const boost::bad_get&)
        {
        }
        catch (const std::invalid_argument&)
        {
        }
      }

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

      escaped_string_parser<Iterator> angle_string;

      qi::symbols<char_type, Tree::InfixAssoc> assoc_symbols;
    };
  }
}

#endif // TL_PARSER_INCLUDED
