/* Equations (ident = expr)
   Copyright (C) 2009--2012 Jarryd Beck

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

#ifndef EQUATION_HPP_INCLUDED
#define EQUATION_HPP_INCLUDED

#include <tl/ast_fwd.hpp>
#include <tl/bestfit.hpp>
#include <tl/exception.hpp>
#include <tl/parser_api.hpp>
#include <tl/types.hpp>
#include <tl/uuid.hpp>
#include <tl/workshop.hpp>

#include <list>
#include <memory>
#include <unordered_map>

/**
 * @file equation.hpp
 * Variables.
 */

namespace TransLucid
{
  class System;

  //represents all definitions of a variable, is responsible for
  //JIT and best fitting
  class VariableWS : public WS, public DefinitionGrouper
  {
    public:
    VariableWS(const u32string& name, System& system);
    
    Constant
    operator()(Context& k)
    {
      return m_bestfit(k);
    }

    Constant
    operator()(Context& kappa, Context& delta)
    {
      return m_bestfit(kappa, delta);
    }

    TimeConstant
    operator()(Context& kappa, Delta& d, const Thread& w, size_t t)
    {
      return m_bestfit(kappa, d, w, t);
    }

    void
    addEquation(uuid id, Parser::RawInput input, int time, 
      ScopePtr = ScopePtr());

    void
    addEquation(uuid id, Parser::Variable eqn, int time, 
      ScopePtr = ScopePtr());

    virtual bool 
    del(uuid id, size_t time);

    virtual bool 
    repl(uuid id, size_t time, Parser::Line line);

    virtual bool 
    repl(uuid id, size_t time, Parser::RawInput line)
    {
      return m_bestfit.repl(id, time, line);
    }

    void
    cache()
    {
      m_bestfit.cache();
    }

    Tree::Expr
    group(const std::list<EquationDefinition>& lines);

    Tree::Expr
    getEquation(Context& k);

    private:
    u32string m_name;

    BestfitGroup m_bestfit;
  };
};

#endif // EQUATION_HPP_INCLUDED
