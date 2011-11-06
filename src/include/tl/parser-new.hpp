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
      Parser(System& system, Context& context);
      
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
      parse_if();

      Token
      nextToken(LexerIterator& begin);

      void
      expect(LexerIterator& begin, const LexerIterator& end, size_t token,
        const u32string& message);

      void
      expect(LexerIterator& begin, const LexerIterator& end, 
        TreeNew::Expr& result,
        bool (Parser::*parser)
          (LexerIterator&, const LexerIterator&, TreeNew::Expr&)
      );

      Context& m_context;
      System::IdentifierLookup m_idents;
    };

    class ParseError : public std::exception
    {
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

      std::string m_message;
    };
  }
}
