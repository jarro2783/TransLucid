/* Translate between representations of code.
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

#ifndef TRANSLATOR_HPP_INCLUDED
#define TRANSLATOR_HPP_INCLUDED

#include <tl/hyperdaton.hpp>
#include <tl/system.hpp>
#include <tl/expr_compiler.hpp>
#include <tl/parser_fwd.hpp>
#include <tl/library.hpp>

namespace TransLucid
{
  namespace Parser
  {
    class Header;

    template <typename Iterator>
    class ExprGrammar;

    template <typename Iterator>
    class EquationGrammar;

    template <typename Iterator>
    class TupleGrammar;

    template <typename Iterator>
    class SkipGrammar;

    template <typename Iterator>
    class HeaderGrammar;
  }

  class Translator
  {
    public:

    Translator();
    ~Translator();

    HD*
    translate_expr(const u32string& s);

    void
    translate_and_add_equation_set(const u32string& s);

    equation_v
    translate_equation_set(const u32string& s);

    /**
     * Add to the header. Parses the input string as a sequence of header
     * items and add them to the header.
     * @param s The header string.
     * @return @b True if the parsing was successful, @b false otherwise.
     */
    bool
    parse_header(const u32string& s);

    HD&
    system()
    {
      return m_system;
    }

    Parser::Header&
    header()
    {
      return *m_header;
    }

    void
    loadLibrary(const u32string& s)
    {
      m_lt.loadLibrary(s, &m_system);
    }

    const Tree::Expr&
    lastExpression() const
    {
      return m_lastExpr;
    }

    private:

    void loadLibraries();

    void cleanup();

    Parser::Header* m_header;

    Parser::ExprGrammar<Parser::iterator_t>* m_expr;
    Parser::EquationGrammar<Parser::iterator_t>* m_equation;
    Parser::TupleGrammar<Parser::iterator_t>* m_tuple;
    Parser::SkipGrammar<Parser::iterator_t>* m_skipper;
    Parser::HeaderGrammar<Parser::iterator_t>* m_header_grammar;

    SystemHD m_system;

    ExprCompiler m_compiler;

    Libtool m_lt;

    Tree::Expr m_lastExpr;

    int m_nextLib;
  };
}

#endif // TRANSLATOR_HPP_INCLUDED
