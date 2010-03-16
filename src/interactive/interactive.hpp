/* TODO: Give a descriptor.
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

#ifndef INTERACTIVE_HPP_INCLUDED
#define INTERACTIVE_HPP_INCLUDED

#include <tl/interpreter.hpp>
#include <tl/parser_fwd.hpp>
#include <tl/parser.hpp>

namespace TLInteractive
{
  namespace TL = TransLucid;
  class System : public TL::Interpreter
  {
    public:

    System()
    : time(0)
    {
      TL::Parser::addSymbol
        (L"demand",
         m_parseInfo.equation_names,
         m_parseInfo.equation_symbols);
    }

    void run();

    void parseHeader(const std::string& file);

    private:
    void postInputSignal(std::vector<TL::AST::Expr*> const& e);
    void postEqnSignal(TL::Parser::equation_v& eqns);
    std::string m_header;
    //TL::EquationSet demands;

    size_t time;
  };
}

#endif // INTERACTIVE_HPP_INCLUDED
