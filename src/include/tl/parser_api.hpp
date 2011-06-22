/* Parser forward declarations.
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

#ifndef PARSER_API_HPP_INCLUDED
#define PARSER_API_HPP_INCLUDED

#include <tl/ast.hpp>
#include <tl/types.hpp>
#include <boost/variant.hpp>
#include <boost/function.hpp>

namespace TransLucid
{
  namespace Parser
  {
    enum DeclType
    {
      DECL_DEF,
      DECL_ASSIGN
    };

    /**
     * A parsed equation.
     * The tuple is defined as: name, | [], & bool, Expr, = or :=
     */
    typedef std::tuple<u32string, Tree::Expr, Tree::Expr, Tree::Expr>
    Equation;

    struct DimensionDecl
    {
      DimensionDecl() = default;

      DimensionDecl(const u32string& d)
      : dim(d)
      {}

      u32string dim;
    };

    struct LibraryDecl
    {
      LibraryDecl() = default;

      LibraryDecl(const u32string& l)
      : lib(l)
      {}

      u32string lib;
    };

    typedef boost::variant
    <
      std::pair<Equation, DeclType>,
      Tree::UnaryOperator,
      Tree::BinaryOperator,
      DimensionDecl,
      LibraryDecl
    > Line;

    typedef std::vector<Line> Instant;

    typedef boost::function<void(Instant)> InstantFunctor;

    /**
     * Prints an equation.
     * Prints to a string.
     * @param e The parsed equation.
     * @return A string representing the equation.
     */
    std::string
    printEquation(const Equation& e);

    class Header;

    template <typename Iterator>
    class ExprGrammar;

    template <typename Iterator>
    class EquationGrammar;

    template <typename Iterator>
    class TupleGrammar;

    template <typename Iterator>
    class HeaderGrammar;

    /**
     * Errors during parsing.
     * Facilitates printing error messages and stores the number of errors
     * that occurred.
     */
    class Errors
    {
      public:
      Errors()
      : m_count(0)
      {
      }

      /**
       * Aids error reporting. 
       * Allows the user to do 
       * errors.error("The number: ")(foo)(" is invalid"). It adds the
       * ability to concatenate any type that has an operator<< in the error
       * report.
       */
      struct ErrorReporter
      {
        /**
         * Create the end error reporter in a list.
         */
        ErrorReporter()
        : m_end(true)
        {
        }

        ~ErrorReporter()
        {
          if (m_end)
          {
            std::cerr << std::endl;
          }
        }

        /**
         * Add more information to the output stream.
         * Executes std::operator<<(std::cerr, t).
         * @param t The object to print to cerr.
         */
        template <typename T>
        ErrorReporter
        operator<<(const T& t)
        {
          m_end = false;
          std::cerr << t;
          return ErrorReporter();
        }

        private:

        template <typename T>
        void
        report(const T& t)
        {
          std::cerr << t;
        }

        bool m_end;
      };

      /**
       * Signify that an error occurred.
       * @param t An object that describes the error, it must have
       * operator<< defined.
       */
      template <typename T>
      ErrorReporter 
      error(const T& t)
      {
        ++m_count;
        return ErrorReporter() << t;
      }

      /**
       * The number of errors that we saw during parsing.
       * @return The number of errors.
       */
      int 
      count()
      {
        return m_count;
      }

      private:
      int m_count;
    };

    inline
    std::ostream&
    operator<<(std::ostream& os, const Equation& eqn)
    {
      os << "Equation(" << std::get<0>(eqn) << ")" << std::endl;
      return os;
    }
    
    inline
    std::ostream&
    operator<<(std::ostream& os, const std::pair<Equation, DeclType>& p)
    {
      os << "Declaration " << p.second << ": " << p.first << std::endl;
      return os;
    }
  }

  typedef std::pair<Parser::Equation, TranslatedEquation> PTEquation;
  typedef std::vector<PTEquation> PTEquationVector;

}

#endif
