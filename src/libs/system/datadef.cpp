/* Data declarations code.
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
 * @file datadef.cpp
 * Data and constructor declarations.
 */

#include <tl/datadef.hpp>
#include <tl/fixed_indexes.hpp>
#include <tl/parser_api.hpp>
#include <tl/system.hpp>

#include <sstream>

namespace TransLucid
{

Constant
ConsDefWS::operator()(Context& k)
{
  return m_bestfit(k);
}

Constant
ConsDefWS::operator()(Context& kappa, Context& delta)
{
  return m_bestfit(kappa, delta);
}

Tree::Expr
ConsDefWS::group(const std::list<EquationDefinition>& defs)
{
  if (defs.size() != 1)
  {
    throw "too many definitions";
  }

  auto& cons = get<Parser::ConstructorDecl>(*defs.front().parsed());

  Tree::TupleExpr::TuplePairs pairs
  {
    {Tree::DimensionExpr{DIM_TYPE}, cons.type},
    {Tree::DimensionExpr{DIM_CONS}, cons.name}
  };

  std::map<u32string, dimension_index> rewrites;
  std::vector<dimension_index> dims;

  for (size_t i = 0; i != cons.args.size(); ++i)
  {
    std::ostringstream argos;
    argos << "arg" << i;
    pairs.push_back(std::make_pair(
      Tree::DimensionExpr(utf8_to_utf32(argos.str())),
      Tree::IdentExpr(cons.args[i])
    ));

    auto dim = m_system.nextHiddenDim();

    dims.push_back(dim);

    rewrites.insert({cons.args[i], dim});
  }

  Tree::Expr guard = fixupGuardArgs(cons.guard, rewrites);

  Tree::ConditionalBestfitExpr cond;
  cond.declarations.push_back(std::make_tuple
  (
    defs.front().start(),
    guard,
    Tree::Expr(),
    Tree::TupleExpr{pairs}
  ));

  Tree::Expr abstractions = cond;

  auto dimIter = dims.rbegin();
  for (auto argsIter = cons.args.rbegin();
       argsIter != cons.args.rend();
       ++argsIter, ++dimIter)
  {
    auto base = Tree::BaseAbstractionExpr(*argsIter, abstractions);
    base.dims.push_back(*dimIter);

    abstractions = std::move(base);
  }

  return abstractions;
}

void
ConsDefWS::addEquation(uuid id, Parser::RawInput input, int time)
{
  m_bestfit.addEquation(id, input, time);
}

bool 
ConsDefWS::del(uuid id, size_t time)
{
  return m_bestfit.del(id, time);
}

bool 
ConsDefWS::repl(uuid id, size_t time, Parser::Line line)
{
  return m_bestfit.repl(id, time, line);
}

}
