/* Core TransLucid application header.
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

#include <tl/parser_api.hpp>
#include <tl/parser_defs.hpp>
#include <tl/system.hpp>
#include <iostream>
#include <tl/ast.hpp>
#include <tl/library.hpp>
//#include <tl/parser_header_util.hpp>
#include <tl/expr_compiler.hpp>

/**
 * @file src/tlcore/tlcore.hpp
 * The main header for tlcore. Contains all the definitions that a main
 * file needs to run the tlcore application.
 */

namespace TransLucid
{
  namespace Parser
  {
    //class ParsedEquation; 
    class Header;
  }

  /**
   * The TLCore namespace. Contains all of the tlcore implementation.
   */
  namespace TLCore
  {
    /**
     * A list of expressions. Stores both the expression tree and the
     * hyperdaton that was generated from it.
     */
    typedef std::vector<std::pair<Tree::Expr, HD*>> ExprList;

    template <typename Iterator>
    class Grammar;

    /**
     * An evaluator for the tlcore syntax. An abstract base class which
     * describes the functionality of a tlcore evaluator.
     */
    class Evaluator
    {
      public:

      /**
       * Add an equation. Informs the system that an equation has been parsed.
       */
      virtual void 
      addEquation(const Parser::ParsedEquation& eqn) = 0;

      /**
       * Add an expression. Informs the system that an expression has been
       * parsed.
       */
      virtual void
      addExpression(const Tree::Expr& e) = 0;

      /**
       * Evaluate an instant.
       * Called after $$ has been input signifying that the next instant can
       * be evaluated.
       */
      virtual void
      evaluateInstant() = 0;

      /**
       * Called after the header is parsed at each instant.
       * Informs the system that new information is to be added to the header.
       * @param header The new header.
       */
      virtual void
      postHeader(const Parser::Header& header) = 0;
    };

    /**
     * The tlcore evaluator.
     */
    class TLCore : public Evaluator
    {
      public:
      /**
       * Construct the evaluator.
       */
      TLCore();

      /**
       * Set verbose.
       * @param v @b true to turn verbose on, @b false to turn it off.
       */
      void 
      verbose(bool v)
      {
        m_verbose = v;
      }

      /**
       * Set reactive.
       * @param r @b true to turn reactive on, @b false to turn it off.
       */
      void 
      reactive(bool r)
      {
        m_reactive = r;
      }

      /**
       * Set demands.
       * @param d @b true to turn demands on, @b false to turn them off.
       */
      void
      demands(bool d)
      {
        m_demands = d;
      }

      /**
       * Set UUIDs. Sets the printing of UUIDs.
       * @param u @b true to turn uuids on, @b false to turn them off.
       */
      void
      uuids(bool u)
      {
        m_uuids = u;
      }

      /**
       * Run the system. Starts the system, parses input and evaluates 
       * according to the semantics of tlcore. Doesn't return until a parse
       * error or end of input.
       */
      void 
      run();

      /**
       * Set the input stream. Sets the stream to use as input.
       * @param is The input stream.
       */
      void 
      set_input(std::istream* is)
      {
        m_is = is;
      }

      /**
       * Set the output stream. Sets the stream to use for output.
       * @param os The output stream.
       */
      void 
      set_output(std::ostream* os)
      {
        m_os = os;
      }

      //adds to the list of equations to add at the end of the instant
      void 
      addEquation(const Parser::ParsedEquation& eqn);

      void
      addExpression(const Tree::Expr& e);

      void 
      evaluateInstant();

      void
      postHeader(const Parser::Header& header);

      private:
      bool m_verbose;
      bool m_reactive;
      bool m_demands;
      bool m_uuids;
      Grammar<Parser::iterator_t>* m_grammar;

      std::istream* m_is;
      std::ostream* m_os;

      SystemHD m_system;
      ExprList m_exprs;
      ExprCompiler m_compiler;

      mpz_class m_time;

      size_t m_lastLibLoaded;

      Libtool m_libtool;

      //name, guard, boolean, expr
      typedef std::tuple<u32string, HD*, HD*, HD*> CompiledEquation;
      std::list<CompiledEquation> m_addEquations;

      std::u32string 
      read_input();

      //does the actual adding to the system
      void
      addNewEquations();
    };
  }
}
