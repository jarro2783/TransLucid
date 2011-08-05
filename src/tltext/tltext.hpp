/* Core TransLucid application header.
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

#include <tl/hyperdatons/arrayhd.hpp>
#include <tl/hyperdatons/envhd.hpp>
#include <tl/ast.hpp>
#include <tl/library.hpp>
#include <tl/parser_api.hpp>
#include <tl/parser_defs.hpp>
#include <tl/system.hpp>
#include <iostream>
//#include <tl/parser_header_util.hpp>
#include <tl/expr_compiler.hpp>

#include "demandhd.hpp"

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

  class LineTokenizer;

  /**
   * The TLText namespace. Contains all of the tlcore implementation.
   */
  namespace TLText
  {
    /**
     * A list of expressions. Stores both the expression tree and the
     * hyperdaton that was generated from it.
     */
    typedef std::vector<std::pair<Tree::Expr, WS*>> ExprList;

    /**
     * The tlcore evaluator.
     */
    class TLText
    {
      public:
      /**
       * Construct the evaluator.
       */
      TLText();

      ~TLText();

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

      void
      set_clargs(const std::vector<std::string>& args)
      {
        m_clargs = args;
      }

      private:
      bool m_verbose;
      bool m_reactive;
      bool m_uuids;

      std::istream* m_is;
      std::ostream* m_os;

      System m_system;
      ExprList m_exprs;
      ExprCompiler m_compiler;

      mpz_class m_time;

      size_t m_lastLibLoaded;

      Libtool m_libtool;

      //name, guard, boolean, expr
      typedef std::tuple<u32string, WS*, WS*, WS*> CompiledEquation;
      std::list<CompiledEquation> m_addEquations;

      //does the actual adding to the system
      void
      addNewEquations();

      //is the instant valid, do we parse expressions
      std::pair<bool, bool>
      processDefinitions(LineTokenizer& line);

      std::vector<Tree::Expr>
      processExpressions(LineTokenizer& line);

      void
      setup_clargs();

      void
      setup_envhd();

      void
      setup_hds();

      DemandHD* m_demands;

      std::vector<std::string> m_headers;
      std::vector<std::string> m_clargs;
      ArrayNHD<u32string, 1>* m_argsHD;
      EnvHD* m_envHD;
    };
  }
}
