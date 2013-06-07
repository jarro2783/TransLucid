/* Dependency checking
   Copyright (C) 2013 Jarryd Beck

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

#include <tl/assignment.hpp>
#include <tl/dependencies.hpp>
#include <tl/output.hpp>

namespace TransLucid
{

namespace Static
{

DependencyFinder::DependencyMap
DependencyFinder::computeDependencies()
{
  typedef std::unordered_map<u32string, Tree::Expr> NameExpr;
  NameExpr exprs;

  m_idDeps.clear();

  DependencyMap currentDeps;

  IdentifierSet demandDeps;

  auto assigns = m_system->getAssignments();

  for (const auto& assign: assigns)
  {
    for (const auto& def : assign.second->definitions())
    {
      auto current = apply_visitor(*this, def.bodyExpr);
      demandDeps.insert(current.first.begin(), current.first.end());
    }
  }

  //compute the dependencies of everything in demandDeps
  std::cout << "demand deps" << std::endl;
  for (const auto& x : demandDeps)
  { auto expr = m_system->getIdentifierTree(x);
    currentDeps[x] = apply_visitor(*this, expr);
    std::cout << x << std::endl;
  }

  m_idDeps = std::move(currentDeps);

  return m_idDeps;
}

DependencyFinder::result_type
DependencyFinder::operator()(const bool& e)
{
  return result_type();
}

DependencyFinder::result_type
DependencyFinder::operator()(const Special& e)
{
  return result_type();
}

DependencyFinder::result_type
DependencyFinder::operator()(const mpz_class& e)
{
  return result_type();
}

DependencyFinder::result_type
DependencyFinder::operator()(const char32_t& e)
{
  return result_type();
}

DependencyFinder::result_type
DependencyFinder::operator()(const u32string& e)
{
  return result_type();
}

DependencyFinder::result_type
DependencyFinder::operator()(const Tree::LiteralExpr& e)
{
  return result_type();
}

DependencyFinder::result_type
DependencyFinder::operator()(const Tree::DimensionExpr& e)
{
  return std::make_pair(IdentifierSet(), 
    FunctorList{Static::Functions::Param{e.dim}});
}

DependencyFinder::result_type
DependencyFinder::operator()(const Tree::IdentExpr& e)
{
  auto iter = m_idDeps.find(e.text);

  if (iter == m_idDeps.end())
  {
    return std::make_pair(IdentifierSet{e.text}, FunctorList{});
  }
  else
  {
    auto idents = iter->second.first;
    idents.insert(e.text);
    return std::make_pair(idents, iter->second.second);
  }
}

DependencyFinder::result_type
DependencyFinder::operator()(const Tree::HashSymbol& e)

{
  throw U"context appears by itself";
}

DependencyFinder::result_type
DependencyFinder::operator()(const Tree::HostOpExpr& e)
{
  return result_type();
}

DependencyFinder::result_type
DependencyFinder::operator()(const Tree::ParenExpr& e)
{
  return apply_visitor(*this, e.e);
}

DependencyFinder::result_type
DependencyFinder::operator()(const Tree::UnaryOpExpr& e)
{
  throw U"error: unary op expression seen in DependencyFinder";
}

DependencyFinder::result_type
DependencyFinder::operator()(const Tree::BinaryOpExpr& e)
{
  throw U"error: binary op expression seen in DependencyFinder";
}

DependencyFinder::result_type
DependencyFinder::operator()(const Tree::MakeIntenExpr& e)
{
  IdentifierSet idents;
  FunctorList funcs;

  for (const auto& expr : e.binds)
  {
    auto deps = apply_visitor(*this, expr);

    idents.insert(deps.first.begin(), deps.first.end());
    funcs.insert(funcs.end(), deps.second.begin(), deps.second.end());
  }

  auto body = apply_visitor(*this, e.expr);

  funcs.push_back(Up{body.first, body.second});

  return result_type(idents, funcs);
}

DependencyFinder::result_type
DependencyFinder::operator()(const Tree::EvalIntenExpr& e)
{
  IdentifierSet resultX;

  auto deps = apply_visitor(*this, e.expr);
  auto eval = Static::Functions::evals_down(deps.second);

  resultX = deps.first;
  resultX.insert(eval.first.begin(), eval.first.end());

  return result_type(resultX, eval.second);
}

DependencyFinder::result_type
DependencyFinder::operator()(const Tree::IfExpr& e)
{
  IdentifierSet X;
  FunctorList F;

  auto cond = apply_visitor(*this, e.condition);
  X.insert(cond.first.begin(), cond.first.end());
  F.insert(F.end(), cond.second.begin(), cond.second.end());

  auto then = apply_visitor(*this, e.then);
  X.insert(then.first.begin(), then.first.end());
  F.insert(F.end(), then.second.begin(), then.second.end());
  
  for (const auto& branch : e.else_ifs)
  {
    auto result = apply_visitor(*this, branch.first);
    X.insert(result.first.begin(), result.first.end());
    F.insert(F.end(), result.second.begin(), result.second.end());

    result = apply_visitor(*this, branch.second);
    X.insert(result.first.begin(), result.first.end());
    F.insert(F.end(), result.second.begin(), result.second.end());
  }

  auto else_ = apply_visitor(*this, e.else_);
  X.insert(else_.first.begin(), else_.first.end());
  F.insert(F.end(), else_.second.begin(), else_.second.end());

  return std::make_pair(X, F);
}

DependencyFinder::result_type
DependencyFinder::operator()(const Tree::HashExpr& e)
{
  auto result = apply_visitor(*this, e.e);

  return std::make_pair(result.first, FunctorList{Static::Functions::Topfun()});
}

DependencyFinder::result_type
DependencyFinder::operator()(const Tree::RegionExpr& e)
{
  IdentifierSet X;
  FunctorList F;

  for (const auto& entry : e.entries)
  {
    auto result = apply_visitor(*this, std::get<0>(entry));
    X.insert(result.first.begin(), result.first.end());
    F.insert(F.end(), result.second.begin(), result.second.end());

    result = apply_visitor(*this, std::get<2>(entry));
    X.insert(result.first.begin(), result.first.end());
    F.insert(F.end(), result.second.begin(), result.second.end());
  }

  return std::make_pair(X, F);
}

DependencyFinder::result_type
DependencyFinder::operator()(const Tree::TupleExpr& e)
{
  IdentifierSet X;
  FunctorList F;

  for (const auto& entry : e.pairs)
  {
    auto result = apply_visitor(*this, entry.first);
    X.insert(result.first.begin(), result.first.end());
    F.insert(F.end(), result.second.begin(), result.second.end());

    result = apply_visitor(*this, entry.second);
    X.insert(result.first.begin(), result.first.end());
    F.insert(F.end(), result.second.begin(), result.second.end());
  }

  return std::make_pair(X, F);
}

DependencyFinder::result_type
DependencyFinder::operator()(const Tree::AtExpr& e)
{
  IdentifierSet X;
  FunctorList F;

  auto result = apply_visitor(*this, e.lhs);
  X.insert(result.first.begin(), result.first.end());
  F.insert(F.end(), result.second.begin(), result.second.end());

  result = apply_visitor(*this, e.rhs);
  X.insert(result.first.begin(), result.first.end());
  F.insert(F.end(), result.second.begin(), result.second.end());

  return std::make_pair(X, F);
}

DependencyFinder::result_type
DependencyFinder::operator()(const Tree::LambdaExpr& e)
{
  IdentifierSet X;
  FunctorList F;

  for (const auto& bind : e.binds)
  {
    auto result = apply_visitor(*this, bind);
    X.insert(result.first.begin(), result.first.end());
    F.insert(F.end(), result.second.begin(), result.second.end());
  }

  auto body = apply_visitor(*this, e.rhs);
  F.push_back(Static::Functions::CBV<IdentifierSet>
    {e.argDim, body.first, body.second});

  return std::make_pair(X, F);
}

DependencyFinder::result_type
DependencyFinder::operator()(const Tree::PhiExpr& e)
{
  throw "cbn function in dependency checker";
}

DependencyFinder::result_type
DependencyFinder::operator()(const Tree::BaseAbstractionExpr& e)
{
  IdentifierSet X;
  FunctorList F;

  for (const auto& bind : e.binds)
  {
    auto result = apply_visitor(*this, bind);
    X.insert(result.first.begin(), result.first.end());
    F.insert(F.end(), result.second.begin(), result.second.end());
  }

  auto body = apply_visitor(*this, e.body);
  F.push_back(Static::Functions::Base<IdentifierSet>{e.dims, 
    body.first, body.second});

  return std::make_pair(X, F);
}

DependencyFinder::result_type
DependencyFinder::operator()(const Tree::BangAppExpr& e)
{
  IdentifierSet X;
  FunctorList F;

  FunctorList F_0;
  std::vector<FunctorList> F_j;
  F_j.reserve(e.args.size());

  auto result = apply_visitor(*this, e.name);
  X.insert(result.first.begin(), result.first.end());
  F_0 = result.second;

  for (const auto& expr : e.args)
  {
    result = apply_visitor(*this, expr);
    X.insert(result.first.begin(), result.first.end());
    F_j.push_back(result.second);
  }

  result = Static::Functions::evals_applyb(F_0, F_j);

  return std::make_pair(X, F);
}

DependencyFinder::result_type
DependencyFinder::operator()(const Tree::LambdaAppExpr& e)
{
  auto lhs = apply_visitor(*this, e.lhs);
  auto rhs = apply_visitor(*this, e.rhs);
  evals_applyv(lhs.second, rhs.second);
  return result_type();
}

DependencyFinder::result_type
DependencyFinder::operator()(const Tree::PhiAppExpr& e)
{
  throw "cbn application in dependency checker";
}

DependencyFinder::result_type
DependencyFinder::operator()(const Tree::WhereExpr& e)
{
  //this is just a wheredim now

  IdentifierSet X;
  FunctorList F;

  auto result = apply_visitor(*this, e.e);
  X.insert(result.first.begin(), result.first.end());
  F.insert(F.end(), result.second.begin(), result.second.end());

  for (const auto& dim : e.dims)
  {
    result = apply_visitor(*this, dim.second);    
    X.insert(result.first.begin(), result.first.end());
    F.insert(F.end(), result.second.begin(), result.second.end());
  }

  return std::make_pair(X, F);
}

DependencyFinder::result_type
DependencyFinder::operator()(const Tree::ConditionalBestfitExpr& e)
{
  IdentifierSet X;
  FunctorList F;

  for (const auto& decl : e.declarations)
  {
    auto result = apply_visitor(*this, std::get<1>(decl));
    X.insert(result.first.begin(), result.first.end());
    F.insert(F.end(), result.second.begin(), result.second.end());

    result = apply_visitor(*this, std::get<2>(decl));
    X.insert(result.first.begin(), result.first.end());
    F.insert(F.end(), result.second.begin(), result.second.end());

    result = apply_visitor(*this, std::get<3>(decl));
    X.insert(result.first.begin(), result.first.end());
    F.insert(F.end(), result.second.begin(), result.second.end());
  }

  return std::make_pair(X, F);
}

} //namespace Static
} //namespace TransLucid
