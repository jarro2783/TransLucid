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

#ifndef TL_FUNCTION_HPP_INCLUDED
#define TL_FUNCTION_HPP_INCLUDED

#include <tl/bestfit.hpp>
#include <tl/parser_api.hpp>
#include <tl/semantics.hpp>
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
      m_bestfit.setName(U"function: " + name);
    }

    Constant
    operator()(Context& k);

    Constant
    operator()(Context& kappa, Context& delta);

    TimeConstant
    operator()(Context& kappa, Delta& d, const Thread& w, size_t t);

    void
    addEquation(uuid id, Parser::RawInput input, int time, 
      ScopePtr = ScopePtr());

    void
    addEquation(uuid id, Parser::Line input, int time,
      ScopePtr scope = ScopePtr())
    {
      m_bestfit.addEquation(id, input, time, scope);
    }

    bool 
    del(uuid id, size_t time);

    bool 
    repl(uuid id, size_t time, Parser::Line line);

    bool 
    repl(uuid id, size_t time, Parser::RawInput line)
    {
      return m_bestfit.repl(id, time, line);
    }

    void
    cache()
    {
      //ignore the request to cache
    }

    Tree::Expr
    group(const std::list<EquationDefinition>& defs);

    Tree::Expr
    getEquation(Context& k)
    {
      return m_bestfit.getEquation(k);
    }

    private:
    u32string m_name;
    System& m_system;
    BestfitGroup m_bestfit;
  };

  Tree::Expr
  fixupGuardArgs(const Tree::Expr& guard,
    const std::map<u32string, dimension_index>& rewrites
  );
}

#endif
