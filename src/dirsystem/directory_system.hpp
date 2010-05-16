/* Directory system.
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

#ifndef DIRECTORY_SYSTEM_HPP_INCLUDED
#define DIRECTORY_SYSTEM_HPP_INCLUDED

#include <tl/interpreter.hpp>
#include <tl/parser_fwd.hpp>
#include <tl/builtin_types.hpp>
#include <tl/expr_compiler.hpp>
#include <tl/utility.hpp>
#include <tl/tuple_parser.hpp>
#include <tl/expr_parser.hpp>

namespace TransLucid
{

  namespace DirectoryParser
  {
    enum FileType
    {
      HEADER,
      EQNS,
      STRUCTURE,
      CLOCK,
      DEMANDS
    };

    class DirectorySystem
    {
      public:
      DirectorySystem();

      bool
      parseSystem(const ustring_t& path);

      template <typename OutputIterator>
      void
      evaluateSystem(OutputIterator out);

      void
      addLibrarySearchPath(const std::string& path);

      private:
      void
      addParsedEquationSet(const Parser::equation_v& eqns);

      void
      setClock(const Parser::equation_v& equations);

      bool
      parseFile(const ustring_t& file, FileType type);

      mpz_class m_maxClock;

      ExprCompiler m_compiler;

      Libtool m_lt;

      TransLucid::SystemHD m_system;

      Parser::HeaderGrammar<Parser::iterator_t> m_header_parser;
      Parser::ExprGrammar<Parser::iterator_t> m_expr_parser;
      Parser::TupleGrammar<Parser::iterator_t> m_tuple_parser;
      Parser::Header m_header;
    };

    template <typename OutputIterator>
    void
    DirectorySystem::evaluateSystem(OutputIterator out)
    {

      using boost::assign::map_list_of;

      //std::cout << "list of variables" << std::endl;
      //listVariables();

      //size_t dimTime = dimTranslator().lookup("time");
      size_t dimTime = DIM_TIME;
      //size_t dim_id = dimTranslator().lookup("id");
      size_t dim_id = DIM_ID;
      //Constant demandString
      //            (String("demand"), typeRegistry().indexString());
      Constant demandString = generate_string(U"demand");

      //evaluate from time 1 to end
      for (size_t time = 1; time <= m_maxClock; ++time)
      {
        //just do one thread
        tuple_t tuple = map_list_of
          (dimTime, Constant(Intmp(time), TYPE_INDEX_INTMP))
          (dim_id, demandString);
        Tuple c(tuple);

        //Equation e = findEquation("demand", c);
        //VariableHD* v = lookupVariable("demand");
        *out = m_system(c);
        #if 0
        VariableHD* v = 0;

        if (v)
        {
          //*out = e.equation()->evaluate(*this, c);
          *out = (*v)(c);
        }
        else
        {
          *out = TaggedConstant(Constant(Special(Special::UNDEF),
                              typeRegistry().indexSpecial()), c);
        }
        #endif
        ++out;
      }
    }
  }
}

#endif // DIRECTORY_SYSTEM_HPP_INCLUDED
