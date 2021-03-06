/* Parser forward declarations.
   Copyright (C) 2009--2012 Jarryd Beck

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

#ifndef PARSER_API_HPP_INCLUDED
#define PARSER_API_HPP_INCLUDED

#include <tl/ast.hpp>
#include <tl/parser_iterator.hpp>
#include <tl/types.hpp>

/**
 * @file parser_api.hpp
 * The definitions needed to use the parser.
 */

namespace TransLucid
{
  namespace Parser
  {
    enum DeclType
    {
      DECL_VARIABLE,
      DECL_ASSIGNMENT,
      DECL_OUTPUT,
      DECL_INPUT,
      DECL_HD
    };

    template <size_t N>
    struct Declaration
    {
      Declaration() = default;

      template <
        typename Eqn, 
        typename Dummy = 
          typename std::enable_if
          <
            std::is_same
            <
              Equation,
              typename std::remove_reference<Eqn>::type
            >::value,
            Eqn
          >::type
      >
      Declaration(Eqn&& e)
      : eqn(std::forward<Eqn>(e))
      {}

      Equation eqn;
    };

    typedef Declaration<DECL_VARIABLE> Variable;
    typedef Declaration<DECL_ASSIGNMENT> Assignment;
    typedef Declaration<DECL_OUTPUT> OutputDecl;
    typedef Declaration<DECL_INPUT> InputDecl;
    typedef Declaration<DECL_HD> HDDecl;

    struct FnDecl
    {
      enum class ArgType
      {
        CALL_BY_BASE,
        CALL_BY_VALUE,
        CALL_BY_NAME
      };

      u32string name;
      std::vector<std::pair<ArgType, u32string>> args;
      Tree::Expr guard;
      Tree::Expr boolean;
      Tree::Expr expr;
    };

    struct DimensionDecl
    {
      DimensionDecl() = default;

      DimensionDecl(const u32string& d)
      : dim(d)
      {}

      template <typename Expression>
      DimensionDecl(const u32string& d, Expression&& expr)
      : dim(d), initialise(std::forward<Expression>(expr))
      {}

      u32string dim;
      Tree::Expr initialise;
    };

    struct LibraryDecl
    {
      LibraryDecl() = default;

      LibraryDecl(const u32string& l)
      : lib(l)
      {}

      u32string lib;
    };

    struct DataConstructor
    {
      u32string name;
      std::vector<DataConstructor> args;
    };

    struct DataType
    {
      u32string name;
      std::vector<u32string> vars;
      std::vector<DataConstructor> constructors;
    };

    struct HostDecl
    {
      u32string identifier;
      Tree::Expr expr;
    };

    struct OpDecl
    {
      u32string optext;
      Tree::Expr expr;
    };

    struct DelDecl
    {
      Tree::Expr id;
    };

    struct ConstructorDecl
    {
      u32string name;
      std::vector<u32string> args;
      Tree::Expr guard;
      u32string type;
    };

    struct ReplDecl;

    typedef Variant
    <
      Variable,
      Assignment,
      //Tree::UnaryOperator,
      //Tree::BinaryOperator,
      DimensionDecl,
      LibraryDecl,
      OutputDecl,
      InputDecl,
      DataType,
      FnDecl,
      HostDecl,
      OpDecl,
      HDDecl,
      DelDecl,
      ConstructorDecl,
      recursive_wrapper<ReplDecl>
    > Line;

    struct ReplDecl
    {
      //replace this...
      std::pair<u32string, u32string> id;
      //with this
      Line repl;
    };
 
    struct Position
    {
      u32string file;
      int line;
      int character;

      U32Iterator begin;
      U32Iterator end;
    };

    struct RawInput
    {
      //where it came from
      u32string source;
      int line;
      int character;

      //the actual input
      u32string text;
    };
     
    class ParseError : public std::exception
    {
      public:
      ParseError(const Position& pos, const std::string& message)
      : m_pos(pos), m_message(message)
      {}

      ~ParseError() noexcept {}

      const char*
      what() const throw()
      {
        return m_message.c_str();
      }

      Position m_pos;
      std::string m_message;
    };

    class ExpectedToken : public ParseError
    {
      public:
      ExpectedToken(const Position& pos, 
        size_t token, const std::string& text);

      ~ExpectedToken() noexcept {}

      size_t
      id() const
      {
        return m_token;
      }

      private:
      Position m_pos;
      std::string m_message;
      size_t m_token;
    };

    class ExpectedExpr : public ParseError
    {
      public:
      ExpectedExpr(const Position& pos, const std::string& text);
    };

    Tree::Expr
    parse_expr(System& system, const u32string& expr);
  }

  typedef std::pair<Parser::Equation, TranslatedEquation> PTEquation;
  typedef std::vector<PTEquation> PTEquationVector;
}

#endif
