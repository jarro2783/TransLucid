/* The parser.
   Copyright (C) 2009, 2010, 2011 Jarryd Beck and John Plaice

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

#include "tl/lexertl.hpp"
#include <tl/ast-new.hpp>
#include <tl/context.hpp>
#include <tl/system.hpp>

#include <exception>

namespace TransLucid
{
  namespace Parser
  {
    class Parser
    {
      public:
      Parser(System& system);
      
      bool
      parse_expr(LexerIterator& begin, const LexerIterator& end,
        TreeNew::Expr& result);

      void
      parse_equation();

      private:
      //all the parse functions
      bool
      parse_where(LexerIterator& begin, const LexerIterator& end,
        TreeNew::Expr& result);

      bool
      parse_binary_op(LexerIterator& begin, const LexerIterator& end,
        TreeNew::Expr& result);

      bool
      parse_app_expr(LexerIterator& begin, const LexerIterator& end,
        TreeNew::Expr& result);

      bool
      parse_token_app(LexerIterator& begin, const LexerIterator& end,
        TreeNew::Expr& result);

      bool
      parse_prefix_expr(LexerIterator& begin, const LexerIterator& end,
        TreeNew::Expr& result);

      bool
      parse_postfix_expr(LexerIterator& begin, const LexerIterator& end,
        TreeNew::Expr& result);

      bool
      parse_if_expr(LexerIterator& begin, const LexerIterator& end,
        TreeNew::Expr& result);

      bool
      parse_primary_expr(LexerIterator& begin, const LexerIterator& end,
        TreeNew::Expr& result);

      void
      parse_function(LexerIterator& begin, const LexerIterator& end,
        TreeNew::Expr& result,
        size_t type);

      Token
      nextToken(LexerIterator& begin);

      void
      expect(LexerIterator& begin, const LexerIterator& end, 
        const u32string& message,
        size_t token
      );

      void
      expect(LexerIterator& begin, const LexerIterator& end, 
        TreeNew::Expr& result, const std::u32string& message,
        bool (Parser::*parser)
          (LexerIterator&, const LexerIterator&, TreeNew::Expr&)
      );

      System::IdentifierLookup m_idents;
      Context& m_context;
    };

    class ParseError : public std::exception
    {
      public:
      ParseError(const std::string& message)
      : m_message(message)
      {}

      const char*
      what() const throw()
      {
        return m_message.c_str();
      }

      protected:
      std::string m_message;
    };

    class ExpectedToken : public ParseError
    {
      public:
      ExpectedToken(size_t token, const u32string& text);

      size_t
      id() const
      {
        return m_token;
      }

      private:
      size_t m_token;
    };

    class ExpectedExpr : public ParseError
    {
      public:
      ExpectedExpr(const u32string& text);
    };
  }
}
