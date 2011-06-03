/* Parser Header.
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
#include <tl/parser_header.hpp>
#include <boost/spirit/include/qi_auxiliary.hpp>
#include <boost/spirit/home/phoenix/object/construct.hpp>

namespace TransLucid
{
  namespace Parser
  {
    namespace ph = boost::phoenix;
    using namespace ph;

    template <typename Iterator>
    class HeaderBinopGrammar : public qi::grammar<Iterator, BinopHeader()>
    {
      public:

      template <typename TokenDef>
      HeaderBinopGrammar(TokenDef& tok)
      : HeaderBinopGrammar::base_type(r_binop)
      {
        using namespace qi::labels;
        r_constant %= tok.constantINTERPRET_ | tok.constantRAW_;

        r_binop = (r_constant > r_constant > tok.integer_
          > tok.dblsemi_)
          [
            _val = ph::bind(&buildBinop, _1, _2, _3)
          ]
        ;

      }
      
      static BinopHeader
      buildBinop
      (
        const std::pair<u32string, u32string>& symbol,
        const std::pair<u32string, u32string>& name,
        const mpz_class& precedence
      )
      {
        if (symbol.first != U"ustring" || name.first != U"ustring")
        {
        }

        return BinopHeader(symbol.second, name.second, precedence);
      }

      qi::rule<Iterator, BinopHeader()> r_binop;
      qi::rule<Iterator, std::pair<u32string, u32string>()> r_constant;
    };

    //a grammar which parses just one constant and accepts just a string
    template <typename Iterator>
    class HeaderStringGrammar :
      public qi::grammar<Iterator, u32string()>
    {
      public:

      template <typename TokenDef>
      HeaderStringGrammar(TokenDef& tok)
      : HeaderStringGrammar::base_type(r_constant)
      {
        using namespace qi::labels;
        r_constant = 
          (tok.constantINTERPRET_ | tok.constantRAW_)
          [
            _val = ph::bind(&buildString, _1)
          ]
        ;
      }

      static u32string
      buildString(const std::pair<u32string, u32string>& literal)
      {
        if (literal.first != U"ustring")
        {
        }

        return literal.second;
      }

      qi::rule<Iterator, u32string()> r_constant;

    };

    /**
     * The header grammar. The grammar required to parse a TransLucid header.
     */
    template <typename Iterator>
    class HeaderGrammar :
      public qi::grammar<Iterator, void(Header&)>
    {
      public:

      /**
       * Construct a HeaderGrammar.
       * The constructor sets up the grammar so that it is ready to parse.
       * @param tok The lexer from which to match tokens.
       */
      template <typename TokenDef>
      HeaderGrammar(TokenDef& tok)
      : HeaderGrammar::base_type(headerp)
      {
         using namespace qi::labels;

         headerp =
           *( headerItem(_r1) > tok.dblsemi_ )
         ;

         headerItem =
           (
             tok.dimension_
               > expr
                 [
                    ph::bind(&addDimension, _r1, _1)
                 ]
           )
         | (
            //infixl,r,n symbol name precedence
               tok.infix_binary_
            >  expr
            >  expr
            >  expr
           )
           [
             ph::bind(&addBinary, _r1, _1, _2, _3, _4)
           ]
         | (
               tok.library_
            > expr
           )
           [
             ph::bind(&addLibraryInternal, _r1, _2)
           ]
         | (
              tok.unary_
           >  expr
           >  expr
           )
           [
             ph::bind(&addUnary, _r1, _1, _2, _3)
           ]
         ;

         integer = qi::int_;

         BOOST_SPIRIT_DEBUG_NODE(headerp);
         BOOST_SPIRIT_DEBUG_NODE(headerItem);
         BOOST_SPIRIT_DEBUG_NODE(expr);
      }

      /**
       * Set the expression parser to use with this header.
       * @param e The expression parser.
       */
      template <class T>
      void
      set_expr(const T& e)
      {
        using namespace qi::labels;
        expr = e[_val = _1];
      }

      private:

      static void
      addDimension
      (
        Header& h,
        const Tree::Expr& name
      )
      {
        try
        {
          const u32string& sname =
            boost::get<const u32string>(name);

          addDimensionSymbol(h, sname);
        } 
        catch (const boost::bad_get&)
        {
        }
      }

      static void
      addBinary
      (
        Header& h,
        const Tree::InfixAssoc& type,
        const Tree::Expr& symbol,
        const Tree::Expr& op,
        const Tree::Expr& precedence
      )
      {
        try
        {
          const u32string& ssymbol =
            boost::get<const u32string>(symbol);
          const u32string& sop =
            boost::get<const u32string>(op);
          const mpz_class& iprecedence =
            boost::get<const mpz_class>(precedence);

          addBinaryOpSymbol
          (
            h,
            ssymbol,
            sop,
            type,
            iprecedence
          );
        }
        catch (const boost::bad_get&)
        {
          throw ParseError(U"Invalid type in binary definition");
        }
      }

      static void
      addUnary
      (
        Header& header,
        Tree::UnaryType type,
        const Tree::Expr& symbol,
        const Tree::Expr& op
      )
      {
        try
        {
          const u32string& csymbol =
            boost::get<const u32string>(symbol);
          const u32string& coperator =
            boost::get<const u32string>(op);

          addUnaryOpSymbol
          (
            header,
            type,
            csymbol,
            coperator
          );
        }
        catch (const boost::bad_get&)
        {
        }
      }

      static void
      addLibraryInternal
      (
        Header& header,
        const Tree::Expr& library
      )
      {
        try
        {
          const u32string& slibrary =
            boost::get<const u32string>(library);

          addLibrary(header, slibrary);
        }
        catch (const boost::bad_get& e)
        {
          std::cerr << "bad_get loading library" << std::endl;
        }
      }

      qi::rule<Iterator, void(Header&)>
        headerp
      ;

      qi::rule<Iterator, void(Header&)>
        headerItem
      ;

      qi::rule<Iterator>
        integer
      ;

      qi::rule<Iterator, Tree::Expr()>
        expr
      ;
    };
  }
}

#endif // TL_PARSER_INCLUDED
