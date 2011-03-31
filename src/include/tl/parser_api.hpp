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

namespace TransLucid
{
  namespace Parser
  {
    //name, | [], & bool, = HD
    typedef std::tuple<u32string, Tree::Expr, Tree::Expr, Tree::Expr>
    ParsedEquation;

    std::string
    printEquation(const ParsedEquation& e);

    class Header;

    template <typename Iterator>
    class ExprGrammar;

    template <typename Iterator>
    class EquationGrammar;

    template <typename Iterator>
    class TupleGrammar;

    template <typename Iterator>
    class HeaderGrammar;

    class Errors
    {
      public:
      Errors()
      : m_count(0)
      {
      }

      struct ErrorReporter
      {
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

      template <typename T>
      ErrorReporter 
      error(const T& t)
      {
        ++m_count;
        return ErrorReporter() << t;
      }

      int 
      count()
      {
        return m_count;
      }

      private:
      int m_count;
    };
  }

}

#endif
