/* Function workshop code
   Copyright (C) 2012 Jarryd Beck

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
 * @file function.hpp
 * Function definition workshop.
 */

#include <tl/bestfit.hpp>
#include <tl/parser_api.hpp>
#include <tl/uuid.hpp>
#include <tl/workshop.hpp>

namespace TransLucid
{
  class System;

  class FunctionWS : public WS, public DefinitionGrouper
  {
    public:

    FunctionWS(const u32string& name, System& system)
    : m_name(name), m_system(system), m_bestfit(this, system)
    {
    }

    Constant
    operator()(Context& k);

    Constant
    operator()(Context& kappa, Context& delta);

    void
    addEquation(uuid id, Parser::RawInput input, int time);

    void
    addEquation(uuid id, Parser::Line input, int time)
    {
      m_bestfit.addEquation(id, input, time);
    }

    bool 
    del(uuid id, size_t time);

    bool 
    repl(uuid id, size_t time, Parser::Line line);

    Tree::Expr
    group(const std::list<EquationDefinition>& defs);

    private:
    u32string m_name;
    System& m_system;
    BestfitGroup m_bestfit;
  };
}
