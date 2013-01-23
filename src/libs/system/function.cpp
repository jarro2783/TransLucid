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
 * @file function.cpp
 * Function workshops.
 */

#include <tl/function.hpp>
#include <tl/system.hpp>

#define STRING(x) #x
#define XSTRING(x) STRING(x)

namespace TransLucid
{

Constant
FunctionWS::operator()(Context& k)
{
  return m_bestfit(k);
}

Constant
FunctionWS::operator()(Context& kappa, Context& delta)
{
  return m_bestfit(kappa, delta);
}

TimeConstant
FunctionWS::operator()(Context& kappa, Delta& d, const Thread& w, size_t t)
{
  return m_bestfit(kappa, d, w, t);
}

void
FunctionWS::addEquation(uuid id, Parser::RawInput input, int time,
  ScopePtr scope)
{
  m_bestfit.addEquation(id, input, time, scope);
}

bool 
FunctionWS::del(uuid id, size_t time)
{
  return m_bestfit.del(id, time);
}

bool 
FunctionWS::repl(uuid id, size_t time, Parser::Line line)
{
  return m_bestfit.repl(id, time, line);
}

Tree::Expr
FunctionWS::group(const std::list<EquationDefinition>& defs)
{
  //for each function definition, rename the parameters in the guard
  //and then build up a conditional bestfit from the resulting expression
  //also make sure that the types of parameters are consistent for all
  //definitions

  std::vector
  <
    std::tuple
    <
      Parser::FnDecl::ArgType,
      u32string,
      dimension_index
    >
  > params;

  Tree::ConditionalBestfitExpr cond;

  cond.name = m_name;

  std::map<u32string, dimension_index> rewrites;

  for (auto& eqn : defs)
  {
    auto& line = *eqn.parsed();
    auto fundecl = get<Parser::FnDecl>(&line);

    if (fundecl == nullptr)
    {
      throw "Internal compiler error: " __FILE__ ":" XSTRING(__LINE__);
    }

    if (params.size() == 0)
    {
      //create the parameters
      for (auto p : fundecl->args)
      {
        auto dim = m_system.nextHiddenDim();
        rewrites.insert({p.second, dim});
        params.push_back(std::make_tuple(p.first, p.second, dim));
      }
    }
    else
    {
      //check that the parameters are consistent
      auto paramIter = params.begin();
      auto declIter = fundecl->args.begin();
      while (declIter != fundecl->args.end() && paramIter != params.end())
      {
        if (std::get<0>(*paramIter) != declIter->first)
        {
          //throw a parse error here
          throw "inconsistent function definition";
        }

        ++declIter;
        ++paramIter;
      }
    }

    auto guardFixed = fixupGuardArgs(fundecl->guard, rewrites);

    //then create the conditional bestfit
    cond.declarations.push_back(
    std::make_tuple(
      eqn.start(),
      guardFixed,
      fundecl->boolean,
      fundecl->expr
    ));
  }

  Tree::Expr abstractions = cond;

  //then build up the function abstractions backwards
  auto iter = params.rbegin();
  while (iter != params.rend())
  {
    if (std::get<0>(*iter) == Parser::FnDecl::ArgType::CALL_BY_NAME)
    {
      auto phi = Tree::PhiExpr(std::get<1>(*iter), abstractions);
      phi.argDim = std::get<2>(*iter);
      abstractions = std::move(phi);
    }
    else if (std::get<0>(*iter) == Parser::FnDecl::ArgType::CALL_BY_BASE)
    {
      auto base = Tree::BaseAbstractionExpr(std::get<1>(*iter), abstractions);
      base.dims.push_back(std::get<2>(*iter));
      abstractions = std::move(base);
    }
    else
    {
      auto lambda = Tree::LambdaExpr(std::get<1>(*iter), abstractions);
      lambda.argDim = std::get<2>(*iter);
      abstractions = std::move(lambda);
    }

    ++iter;
  }

  return abstractions;
}

Tree::Expr
fixupGuardArgs(const Tree::Expr& guard,
  const std::map<u32string, dimension_index>& rewrites
)
{
  //first check if it is nil
  const Tree::nil* n = get<Tree::nil>(&guard);
  if (n != nullptr)
  {
    return guard;
  }

  //the guard must be a region
  const Tree::RegionExpr& region = get<Tree::RegionExpr>(guard);

  decltype(region.entries) rewritten;

  for (auto& p : region.entries)
  {
    const Tree::IdentExpr* id = get<Tree::IdentExpr>(&std::get<0>(p));
    if (id != nullptr)
    {
      auto iter = rewrites.find(id->text);
      if (iter != rewrites.end())
      {
        rewritten.push_back(std::make_tuple(
          Tree::DimensionExpr(iter->second), std::get<1>(p), std::get<2>(p)));
      }
      else
      {
        rewritten.push_back((p));
      }
    }
    else
    {
      rewritten.push_back((p));
    }
  }

  return Tree::RegionExpr{rewritten};
}

}
