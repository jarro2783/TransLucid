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

#include <tl/ast.hpp>
#include <tl/hyperdatons/multi_arrayhd_fwd.hpp>
#include <tl/hyperdatons/envhd.hpp>
#include <tl/library.hpp>
#include <tl/system.hpp>
#include <iostream>

#include "demandhd.hpp"

/**
 * @file src/tltext/tltext.hpp
 * The main header for tltext. Contains all the definitions that a main
 * file needs to run the tltext application.
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
      TLText(const std::string& initOut);

      ~TLText();

      /**
       * Set verbose.
       * @param v @b true to turn verbose on, @b false to turn it off.
       */
      void 
      verbose(int v)
      {
        m_verbose = v;
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

      void
      debug(bool d = true)
      {
        m_debug = d;
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
      set_input(std::istream* is, const std::string& name);

      template <typename String>
      void
      add_input(String&& file, std::istream* is);

      /**
       * Set the output stream. Sets the stream to use for output.
       * @param os The output stream.
       */
      void 
      set_output(std::ostream* os)
      {
        m_os = os;
      }

      /**
       * Set the error output stream. Sets the stream to use for error output.
       * @param os The error output stream.
       */
      void
      set_error(std::ostream* os)
      {
        m_error = os;
      }

      /**
       * Set the command line arguments.
       * @param args The command line arguments.
       */
      void
      set_clargs(const std::vector<std::string>& args)
      {
        m_clargs = args;
      }

      /**
       * Add a header file. Adds a header file to load up before starting.
       * @param header The header to load.
       */
      void
      add_header(const std::string& header);

      void
      add_argument(const u32string& arg);

      void
      add_argument(const u32string& arg, const u32string& value);

      private:
      int m_verbose;
      bool m_uuids;
      bool m_debug;

      std::istream* m_is;
      std::ostream* m_os;
      std::ostream* m_error;

      u32string m_inputName;

      std::string m_initialOut;

      System m_system;
      ExprList m_exprs;

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
      processDefinitions(LineTokenizer& line, const u32string& streamName);

      std::vector<Tree::Expr>
      processExpressions(LineTokenizer& line, const u32string& streamName);

      void
      setup_clargs();

      void
      setup_envhd();

      void
      setup_hds();

      class VerboseOutput
      {
        public:
        VerboseOutput(int currentLevel, int wantedLevel, std::ostream& os)
        : m_current(currentLevel), m_wanted(wantedLevel), m_os(os)
        {
        }

        int m_current;
        int m_wanted;
        std::ostream& m_os;
      };

      VerboseOutput&
      output(std::ostream& os, const u32string& text);

      DemandHD* m_demands;
      DemandHD* m_returnhd;

      std::vector<std::string> m_headers;
      std::vector<std::string> m_clargs;
      ArrayNHD<u32string, 1>* m_argsHD;
      EnvHD* m_envHD;
    };

    enum TLtextReturnCode
    {
      RETURN_CODE_NOT_INTMP = 1,
      RETURN_CODE_BOUNDS
    };

    class ReturnError : public std::exception
    {
      public:
      ReturnError(int code)
      : m_code(code)
      {
      }

      const char* what() const throw()
      {
        return "TransLucid TLtext return code exception";
      }

      int m_code;
    };
  }
}
