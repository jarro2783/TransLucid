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

/**
 * @file translator.hpp
 * Header file for the translator class.
 */

#ifndef TRANSLATOR_HPP_INCLUDED
#define TRANSLATOR_HPP_INCLUDED

#include <tl/expr_compiler.hpp>
#include <tl/workshop.hpp>
#include <tl/library.hpp>
#include <tl/parser_api.hpp>
#include <tl/types.hpp>
#include <tl/parser_iterator.hpp>

#include <boost/function.hpp>

namespace TransLucid
{
  namespace Lexer
  {
    template <typename Lexer>
    class lex_tl_tokens;
  }

  namespace detail
  {
    class AllParsers;
  }

  class System;

  /**
   * Translator class. Drives the parser.
   */
  class Translator
  {
    private:
    typedef std::map<uuid, Parser::Equation> UUIDToEquation;

    public:

    /**
     * An iterator into a list of equations. EquationIterator is a façade
     * to UUIDToEquation.
     */
    class EquationIterator
    {
      public:
      /**
       * Construct an EquationIterator.
       * @param iter The underlying iterator.
       */
      EquationIterator(const UUIDToEquation::const_iterator& iter)
      : m_iter(iter)
      {
      }

      /**
       * Copies an EquationIterator.
       * @param other The iterator to copy.
       */
      EquationIterator(const EquationIterator& other)
      : m_iter(other.m_iter)
      {
      }

      /**
       * Assigns an EquationIterator.
       * @param rhs The iterator to assign to.
       * @return *this.
       */
      EquationIterator& operator=(const EquationIterator& rhs)
      {
        if (&rhs != this)
        {
          m_iter = rhs.m_iter;
        }
        return *this;
      }

      /**
       * Tests equality to another iterator.
       * @param rhs The other iterator.
       * @return True if equal, false if not.
       */
      bool operator==(const EquationIterator& rhs) const
      {
        return m_iter == rhs.m_iter;
      }

      /**
       * Tests inequality.
       * @param rhs The other iterator.
       * @return True if not equal, false if equal.
       */
      bool operator!=(const EquationIterator& rhs) const
      {
        return m_iter != rhs.m_iter;
      }

      /**
       * Preincrement the iterator.
       * @return *this.
       */
      EquationIterator& operator++() 
      {
        ++m_iter;
        return *this;
      }

      /**
       * Postincrement operator.
       * return A copy of the iterator before incrementing.
       */
      EquationIterator operator++(int)
      {
        EquationIterator i(m_iter);
        ++m_iter;

        return i;
      }

      /**
       * Get the current uuid.
       * @return The uuid of the equation currently being referred to.
       */
      uuid id() const
      {
        return m_iter->first;
      }

      /**
       * Print the current equation.
       * @return A string representation of the current equation.
       */
      std::string print() const;

      private:
      UUIDToEquation::const_iterator m_iter;
    };

    Translator(System& system);
    ~Translator();

    #if 0
    /**
     * Translate an expression into a hyperdaton.
     * Parses and compiles an expression string.
     * @param s The string representing the expression.
     */
    WS*
    translate_expr(const u32string& s);
    #endif

    /**
     * Add to the header. Parses the input string as a sequence of header
     * items and add them to the header.
     * @param s The header string.
     * @return @b True if the parsing was successful, @b false otherwise.
     */
    bool
    parse_header(const u32string& s);

    std::pair<bool, std::pair<Parser::Equation, Parser::DeclType>>
    parseEquation
    (
      Parser::U32Iterator& begin, 
      const Parser::U32Iterator& end
    );

    std::pair<bool, Parser::BinopHeader>
    parseHeaderBinary
    (
      Parser::U32Iterator& begin, 
      const Parser::U32Iterator& end
    );

    std::pair<bool, u32string>
    parseHeaderString
    (
      Parser::U32Iterator& begin, 
      const Parser::U32Iterator& end
    );

    std::pair<bool, Parser::UnopHeader>
    parseHeaderUnary
    (
      Parser::U32Iterator& begin, 
      const Parser::U32Iterator& end
    );

    bool
    parseInstant
    (
      Parser::U32Iterator& begin,
      const Parser::U32Iterator& end,
      InstantFunctor endInstant
    );

    /**
     * The Translator's system.
     * @return The System belonging to the Translator.
     */
    System&
    system()
    {
      return m_system;
    }

    /**
     * The Translator's header.
     * The parser header belonging to the Translator.
     */
    Parser::Header&
    header()
    {
      return *m_header;
    }

    /**
     * Load a library. Loads the library given by the string @a s.
     * @param s The name of the library to load.
     */
    void
    loadLibrary(const u32string& s);

    /**
     * The AST of the last expression parsed. When translate_expr is called,
     * the expression tree will be stored and can be retrieved by this
     * function.
     * @return The expression tree of the last expression parsed by
     * translate_expr.
     */
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

    detail::AllParsers* m_parsers;

    System& m_system;

    ExprCompiler m_compiler;

    Libtool m_lt;

    Tree::Expr m_lastExpr;
    UUIDToEquation m_uuidParsedEqns;

    unsigned int m_nextLib;
  };
}

#endif // TRANSLATOR_HPP_INCLUDED
