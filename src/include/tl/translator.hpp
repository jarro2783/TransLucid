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

#include <tl/expr_compiler.hpp>
#include <tl/hyperdaton.hpp>
#include <tl/library.hpp>
#include <tl/parser_fwd.hpp>
#include <tl/system.hpp>
#include <tl/types.hpp>

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

  typedef std::pair<Parser::ParsedEquation, TranslatedEquation> PTEquation;
  typedef std::vector<PTEquation> PTEquationVector;

  class Translator
  {
    private:
    typedef std::map<uuid, Parser::ParsedEquation> UUIDToParsedEquation;

    public:

    class EquationIterator
    {
      public:
      EquationIterator(const UUIDToParsedEquation::const_iterator& iter)
      : m_iter(iter)
      {
      }

      EquationIterator(const EquationIterator& other)
      : m_iter(other.m_iter)
      {
      }

      EquationIterator& operator=(const EquationIterator& rhs)
      {
        if (&rhs != this)
        {
          m_iter = rhs.m_iter;
        }
        return *this;
      }

      bool operator==(const EquationIterator& rhs) const
      {
        return m_iter == rhs.m_iter;
      }

      bool operator!=(const EquationIterator& rhs) const
      {
        return m_iter != rhs.m_iter;
      }

      EquationIterator& operator++() 
      {
        ++m_iter;
        return *this;
      }

      EquationIterator operator++(int)
      {
        EquationIterator i(m_iter);
        ++m_iter;

        return i;
      }

      uuid id() const
      {
        return m_iter->first;
      }

      std::string print() const;

      private:
      UUIDToParsedEquation::const_iterator m_iter;
    };

    Translator();
    ~Translator();

    HD*
    translate_expr(const u32string& s);

    void
    translate_and_add_equation_set(const u32string& s);

    PTEquationVector
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

    /**
     * Print a single equation. Prints the equation with the uuid @id if
     * it was added to the system by this Translator object.
     * @param id The equation uuid.
     * @return A textual representation of the equation.
     */
    std::string
    printEquation(const uuid& id) const;

    /**
     * Retrieve an iterator to the beginning of the equations.
     * The equations will be sorted by their uuids.
     * @return An EquationIterator object which can iterator through all the
     * equations added so far.
     */
    EquationIterator
    beginEquations() const
    {
      return EquationIterator(m_uuidParsedEqns.begin());
    }

    /**
     * Retrieve an iterator to the end of the equations. Use with 
     * beginEquations to determine when to stop iterating.
     * @return An end iterator.
     */
    EquationIterator
    endEquations() const
    {
      return EquationIterator(m_uuidParsedEqns.end());
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
    UUIDToParsedEquation m_uuidParsedEqns;

    unsigned int m_nextLib;
  };
}

#endif // TRANSLATOR_HPP_INCLUDED
