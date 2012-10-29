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
 * @file opdef.cpp
 * Operator declarations implementation.
 */

#include <tl/function.hpp>
#include <tl/opdef.hpp>
#include <tl/system.hpp>

namespace TransLucid
{

Constant
OpDefWS::operator()(Context& k)
{
  return m_bestfit(k);
}

Constant
OpDefWS::operator()(Context& kappa, Context& delta)
{
  return m_bestfit(kappa, delta);
}

bool 
OpDefWS::del(uuid id, size_t time)
{
  return m_bestfit.del(id, time);
}

bool 
OpDefWS::repl(uuid id, size_t time, Parser::Line line)
{
  return m_bestfit.repl(id, time, line);
}

void
OpDefWS::addEquation(uuid id, Parser::RawInput input, int time)
{
  m_bestfit.addEquation(id, input, time);
}

void
OpDefWS::addEquation(uuid id, Parser::Line input, int time)
{
  m_bestfit.addEquation(id, input, time);
}

Tree::Expr
OpDefWS::group(const std::list<EquationDefinition>& defs)
{
  //build a function abstraction
  // \op -> bestof defs

  Tree::ConditionalBestfitExpr best;

  auto dim = m_system.nextHiddenDim();

  std::map<u32string, dimension_index> rewrites{{U"op", dim}};

  for (auto eqn : defs)
  {
    auto decl = get<Parser::OpDecl>(&*eqn.parsed());

    if (decl == nullptr)
    {
      throw "operator definition not an operator";
    }

    auto guard = Tree::RegionExpr(Tree::RegionExpr::Entries
      {
        Tree::RegionExpr::Entry{
          Tree::IdentExpr(U"op"), 
          Region::Containment::IS, 
          decl->optext
        }
      });
    auto guardFixed = fixupGuardArgs(guard, rewrites);

    best.declarations.push_back
    (
      std::make_tuple
      (
        eqn.start(),
        guardFixed,
        Tree::Expr(),
        decl->expr
      )
    );
  }

  Tree::BaseAbstractionExpr fn(U"op", best);
  fn.dims.push_back(dim);

  return fn;
}

}
