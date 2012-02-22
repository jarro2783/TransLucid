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

#ifndef TL_PARSER_HPP_INCLUDED
#define TL_PARSER_HPP_INCLUDED

#include <exception>
#include <unordered_map>

#include "tl/lexertl.hpp"
#include <tl/ast.hpp>
#include <tl/context.hpp>
#include <tl/system.hpp>

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
        Tree::Expr& result);

      bool
      parse_line(LexerIterator& begin, const LexerIterator& end,
        Line& result);

      private:
      enum TupleSeparator
      {
        SEPARATOR_COLON,
        SEPARATOR_ARROW
      };

      //all the parse functions
      bool
      parse_where(LexerIterator& begin, const LexerIterator& end,
        Tree::Expr& result);

      bool
      parse_binary_op(LexerIterator& begin, const LexerIterator& end,
        Tree::Expr& result);

      bool
      parse_app_expr(LexerIterator& begin, const LexerIterator& end,
        Tree::Expr& result);

      bool
      parse_token_app(LexerIterator& begin, const LexerIterator& end,
        Tree::Expr& result);

      bool
      parse_prefix_expr(LexerIterator& begin, const LexerIterator& end,
        Tree::Expr& result);

      bool
      parse_postfix_expr(LexerIterator& begin, const LexerIterator& end,
        Tree::Expr& result);

      bool
      parse_if_expr(LexerIterator& begin, const LexerIterator& end,
        Tree::Expr& result);

      bool
      parse_primary_expr(LexerIterator& begin, const LexerIterator& end,
        Tree::Expr& result);

      void
      parse_function(LexerIterator& begin, const LexerIterator& end,
        Tree::Expr& result,
        size_t type);

      bool 
      parse_tuple(LexerIterator& begin, const LexerIterator& end,
        Tree::Expr& result, TupleSeparator sep);

      bool
      parse_equation_decl(LexerIterator& begin, const LexerIterator& end,
        Equation& result, size_t separator_symbol, 
        const std::string& separator_text);

      bool
      parse_host_decl(LexerIterator& begin, const LexerIterator& end,
        Line& result);

      bool
      parse_dim_decl(LexerIterator& begin, const LexerIterator& end,
        Line& result);

      bool
      parse_assign_decl(LexerIterator& begin, const LexerIterator& end,
        Line& result);

      bool
      parse_var_decl(LexerIterator& begin, const LexerIterator& end,
        Line& result);

      bool
      parse_out_decl(LexerIterator& begin, const LexerIterator& end,
        Line& result);

      bool
      parse_in_decl(LexerIterator& begin, const LexerIterator& end,
        Line& result);

      bool
      parse_del_decl(LexerIterator& begin, const LexerIterator& end,
        Line& result);

      bool
      parse_hd_decl(LexerIterator& begin, const LexerIterator& end,
        Line& result);

      bool
      parse_equation_decl(LexerIterator& begin, const LexerIterator& end,
        Equation& result);

      //checks that begin is a ustring"stuff" and puts stuff in result
      bool
      is_string_constant(LexerIterator& begin, const LexerIterator& end,
        u32string& result);

      bool
      parse_data_decl(LexerIterator& begin, const LexerIterator& end,
        Line& result);

      bool
      parse_fun_decl(LexerIterator& begin, const LexerIterator& end,
        Line& result);

      bool
      parse_data_constructor(LexerIterator& begin, const LexerIterator& end,
        DataConstructor& result);

      bool
      parse_op_decl(LexerIterator& begin, const LexerIterator& end,
        Line& result);

      Token
      nextToken(LexerIterator& begin);

      void
      expect_no_advance(LexerIterator& begin, const LexerIterator& end, 
        const std::string& message,
        size_t token
      );

      void
      expect(LexerIterator& begin, const LexerIterator& end, 
        const std::string& message,
        size_t token
      );

#if 0
      template <typename... T>
      void
      expect(LexerIterator& begin, const LexerIterator& end, 
        Tree::Expr& result, const std::u32string& message,
        bool (Parser::*parser)
          (LexerIterator&, const LexerIterator&, Tree::Expr&, T...),
        T&&... args
      );
#endif

      template <typename Result, typename Fn, typename... Args>
      void
      expect(LexerIterator& begin, const LexerIterator& end, 
        Result&& result, const std::string& message,
        Fn f,
        Args&&... args
      );

      System& m_system;
      System::IdentifierLookup m_idents;
      Context& m_context;

      //the decl parsers
      typedef 
      std::unordered_map
      <
        u32string, 
        std::function<bool(LexerIterator&, const LexerIterator, Line&)>
      > DeclParsers;

      DeclParsers m_where_decls;
      DeclParsers m_top_decls;

      std::stack<DeclParsers*> m_which_decl;
    };
  }
}

#endif
