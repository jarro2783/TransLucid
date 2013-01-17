/* Op declarations code.
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
 * @file opdef.hpp
 * Operator declarations.
 */

#ifndef TL_OPDEF_HPP_INCLUDED
#define TL_OPDEF_HPP_INCLUDED

#include <tl/bestfit.hpp>
#include <tl/workshop.hpp>

namespace TransLucid
{
  class OpDefWS : public WS, public DefinitionGrouper
  {
    public:

    OpDefWS(System& system)
    : m_bestfit(this, system), m_system(system)
    {
    }

    Constant
    operator()(Context& k);

    Constant
    operator()(Context& kappa, Context& delta);

    TimeConstant
    operator()(Context& kappa, Delta& d, const Thread& w, size_t t);

    void
    addEquation(uuid id, Parser::Line input, int time);

    void
    addEquation(uuid id, Parser::RawInput input, int time);

    bool 
    del(uuid id, size_t time);

    bool 
    repl(uuid id, size_t time, Parser::Line line);

    bool 
    repl(uuid id, size_t time, Parser::RawInput line)
    {
      return m_bestfit.repl(id, time, line);
    }

    //ignore the request to cache
    void
    cache()
    {
    }

    Tree::Expr
    group(const std::list<EquationDefinition>& defs);

    private:

    BestfitGroup m_bestfit;
    System& m_system;
  };
}

#endif
