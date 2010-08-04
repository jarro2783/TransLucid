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

#include <tl/parser_fwd.hpp>
#include <tl/system.hpp>
#include <iostream>
#include <tl/ast.hpp>
#include <tl/library.hpp>
#include <tl/parser_header_util.hpp>

namespace TransLucid
{
  namespace Parser
  {
    template <typename Iterator>
    class SkipGrammar;
  }

  namespace TLCore
  {
    typedef std::vector<std::pair<Tree::Expr, HD*>> ExprList;

    template <typename Iterator>
    class Grammar;

    class Evaluator
    {
      public:

      virtual void 
      addEquation(const Parser::ParsedEquation& eqn) = 0;

      virtual void
      addExpression(const Tree::Expr& e) = 0;

      virtual void
      evaluateInstant() = 0;

      virtual void
      postHeader(const Parser::Header& header) = 0;
    };

    class TLCore : public Evaluator
    {
      public:
      TLCore();

      void 
      verbose(bool v)
      {
        m_verbose = v;
      }

      void 
      reactive(bool r)
      {
        m_reactive = r;
      }

      void
      demands(bool d)
      {
        m_demands = d;
      }

      void 
      run();

      void 
      set_input(std::istream* is)
      {
        m_is = is;
      }

      void 
      set_output(std::ostream* os)
      {
        m_os = os;
      }

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
      Grammar<Parser::iterator_t>* m_grammar;
      Parser::SkipGrammar<Parser::iterator_t>* m_skipper;      

      std::istream* m_is;
      std::ostream* m_os;

      SystemHD m_system;
      ExprList m_exprs;

      mpz_class m_time;

      size_t m_lastLibLoaded;

      Libtool m_libtool;

      std::u32string 
      read_input();
    };
  }
}
