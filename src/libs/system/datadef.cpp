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

  for (size_t i = 0; i != cons.args.size(); ++i)
  {
    std::ostringstream argos;
    argos << "arg" << i;
    pairs.push_back(std::make_pair(
      Tree::DimensionExpr(utf8_to_utf32(argos.str())),
      Tree::IdentExpr(cons.args[i])
    ));
  }

  Tree::ConditionalBestfitExpr cond;
  cond.declarations.push_back(std::make_tuple
  (
    defs.front().start(),
    cons.guard,
    Tree::Expr(),
    Tree::TupleExpr{pairs}
  ));

  Tree::Expr abstractions = cond;

  for (auto iter = cons.args.rbegin(); iter != cons.args.rend(); ++iter)
  {
    abstractions = Tree::LambdaExpr(*iter, abstractions);
  }
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
}

}
