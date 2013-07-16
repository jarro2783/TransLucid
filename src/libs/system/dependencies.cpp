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

#include <algorithm>

#include <tl/assignment.hpp>
#include <tl/dependencies.hpp>
#include <tl/static/function_printer.hpp>
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

  IdentifierSet seenVars;
  IdentifierSet currentSeenVars;

  size_t numObjects = 0;
  size_t currentNumObjects = 0;

  auto assigns = m_system->getAssignments();

  for (const auto& assign: assigns)
  {
    for (const auto& def : assign.second->definitions())
    {
      auto current = apply_visitor(*this, def.bodyExpr);
      currentSeenVars.insert(std::get<0>(current).begin(), 
        std::get<0>(current).end());

      currentNumObjects += Static::Functions::count_objects(
        std::get<1>(current));
    }
  }

  //std::cout << "After demands, number of objects: " 
  //  << currentNumObjects << std::endl;

  //compute the dependencies of everything in seenVars until a 
  //least-fixed point
  int i = 0;
  while (currentSeenVars.size() != seenVars.size()
    || numObjects != currentNumObjects)
  {
    numObjects = currentNumObjects;
    currentNumObjects = 0;
    seenVars = currentSeenVars;

    for (const auto& x : seenVars)
    { 
      try
      {
        auto expr = m_system->getIdentifierTree(x);
        auto& deps = currentDeps[x] = apply_visitor(*this, expr);

        for (const auto& f : std::get<1>(deps))
        {
          Static::Functions::collect_properties(
            f, currentSeenVars);
          currentNumObjects += Static::Functions::count_objects(f);
        }

        for (const auto& f : std::get<2>(deps))
        {
          Static::Functions::collect_properties(
            f, currentSeenVars);
          currentNumObjects += Static::Functions::count_objects(f);
        }

      }
      catch (const u32string& e)
      {
        std::cerr << "exception checking dependencies of '" << x << "':\n"
          << e << std::endl;
        throw;
      }
    }

// temporary printing code
    #if 0
    std::cout << "Iteration " << i << std::endl;
    for (const auto& dep : currentDeps)
    {
      std::cout << dep.first << ": ";

      for (const auto& x : std::get<0>(dep.second))
      {
        std::cout << x << " ";
      }
      std::cout << std::endl;
      
      print_container(std::cout, std::get<1>(dep.second));
      std::cout << std::endl;
    }
    std::cout << "End iteration " << i << std::endl;
    #endif

// end temporary printing code

    //std::cout << "Number of objects: " << currentNumObjects << std::endl;
    //std::cout << "Number of seen vars: " << currentSeenVars.size() 
    //  << std::endl;
    m_idDeps = std::move(currentDeps);
    ++i;
  }

  //std::cout << "took " << i << " iterations to compute dependencies" << 
  //  std::endl;

  //compute the dependencies of the demands one last time

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
  return std::make_tuple(IdentifierSet(), 
    FunctorList{Static::Functions::Param{e.dim}}, FunctorList{});
}

DependencyFinder::result_type
DependencyFinder::operator()(const Tree::IdentExpr& e)
{
  auto iter = m_idDeps.find(e.text);

  if (iter == m_idDeps.end())
  {
    return std::make_tuple(IdentifierSet{e.text}, FunctorList{}, FunctorList{});
  }
  else
  {
    auto idents = std::get<0>(iter->second);
    idents.insert(e.text);
    return std::make_tuple(idents, std::get<1>(iter->second), 
      std::get<2>(iter->second));
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
  FunctorList fundeps;

  for (const auto& expr : e.binds)
  {
    auto deps = apply_visitor(*this, expr);

    idents.insert(std::get<0>(deps).begin(), std::get<0>(deps).end());

    //TODO must insert the ones that are applications into \Fcal
    std::copy_if(std::get<1>(deps).begin(), std::get<1>(deps).end(),
      std::inserter(fundeps, fundeps.end()), Static::Functions::is_app());

    fundeps.insert(fundeps.end(), std::get<2>(deps).begin(), 
      std::get<2>(deps).end());
  }

  auto body = apply_visitor(*this, e.expr);

  funcs.push_back(Up{std::get<0>(body), std::get<1>(body), std::get<2>(body)});

  return result_type(idents, funcs, fundeps);
}

DependencyFinder::result_type
DependencyFinder::operator()(const Tree::EvalIntenExpr& e)
{
  IdentifierSet resultX;
  FunctorList resultF;
  FunctorList fundeps;

  auto deps = apply_visitor(*this, e.expr);

  FunctorList ups;

  for (const auto& f : std::get<1>(deps))
  {
    auto fup = get<Up>(&f);

    if (fup == nullptr)
    {
      resultF.push_back(Down{f});
    }
    else
    {
      ups.push_back(f);
    }
  }

  auto eval = Static::Functions::evals_down(ups);

  fundeps = std::get<2>(deps);

  fundeps.insert(fundeps.end(), std::get<2>(eval).begin(), 
    std::get<2>(eval).end());

  resultF.insert(resultF.end(), std::get<1>(eval).begin(), 
    std::get<1>(eval).end());

  resultX = std::get<0>(deps);
  resultX.insert(std::get<0>(eval).begin(), std::get<0>(eval).end());

  return result_type(resultX, resultF, fundeps);
}

DependencyFinder::result_type
DependencyFinder::operator()(const Tree::IfExpr& e)
{
  IdentifierSet X;
  FunctorList F;
  FunctorList Fcal;

  //copy the apps from the F result into Fcal
  auto cond = apply_visitor(*this, e.condition);
  X.insert(std::get<0>(cond).begin(), std::get<0>(cond).end());
  std::copy_if(std::get<1>(cond).begin(), std::get<1>(cond).end(), 
    std::inserter(Fcal, Fcal.end()), Static::Functions::is_app());
  Fcal.insert(Fcal.end(), std::get<2>(cond).begin(), std::get<2>(cond).end());

  //these ones are just passed along
  auto then = apply_visitor(*this, e.then);
  X.insert(std::get<0>(then).begin(), std::get<0>(then).end());
  F.insert(F.end(), std::get<1>(then).begin(), std::get<1>(then).end());
  Fcal.insert(Fcal.end(), std::get<2>(then).begin(), std::get<2>(then).end());
  
  for (const auto& branch : e.else_ifs)
  {
    //as above, copy the apps from the F result into Fcal
    auto result = apply_visitor(*this, branch.first);
    X.insert(std::get<0>(result).begin(), std::get<0>(result).end());
    std::copy_if(std::get<1>(result).begin(), std::get<1>(result).end(), 
      std::inserter(Fcal, Fcal.end()), Static::Functions::is_app());
    Fcal.insert(
      Fcal.end(), std::get<2>(result).begin(), std::get<2>(result).end());

    //these ones are just passed along
    result = apply_visitor(*this, branch.second);
    X.insert(std::get<0>(result).begin(), std::get<0>(result).end());
    F.insert(F.end(), std::get<1>(result).begin(), std::get<1>(result).end());
    Fcal.insert(Fcal.end(), std::get<2>(result).begin(), 
      std::get<2>(result).end());
  }

  auto else_ = apply_visitor(*this, e.else_);
  X.insert(std::get<0>(else_).begin(), std::get<0>(else_).end());
  F.insert(F.end(), std::get<1>(else_).begin(), std::get<1>(else_).end());
  Fcal.insert(Fcal.end(), std::get<2>(else_).begin(), 
    std::get<2>(else_).end());

  return std::make_tuple(X, F, Fcal);
}

DependencyFinder::result_type
DependencyFinder::operator()(const Tree::HashExpr& e)
{

  auto dimbody = get<Tree::DimensionExpr>(&e.e);

  if (dimbody != nullptr)
  {
    return std::make_tuple(IdentifierSet(), 
      FunctorList{Static::Functions::Param{dimbody->dim}}, FunctorList{});
  }

  auto result = apply_visitor(*this, e.e);

  return std::make_tuple(
    std::get<0>(result), 
    FunctorList{Static::Functions::Topfun()},
    std::get<2>(result)
  );
}

DependencyFinder::result_type
DependencyFinder::operator()(const Tree::RegionExpr& e)
{
  IdentifierSet X;
  FunctorList Fcal;

  for (const auto& entry : e.entries)
  {
    auto result = apply_visitor(*this, std::get<0>(entry));
    X.insert(std::get<0>(result).begin(), std::get<0>(result).end());
    std::copy_if(std::get<1>(result).begin(), std::get<1>(result).end(),
      std::inserter(Fcal, Fcal.end()), Static::Functions::is_app());
    Fcal.insert(Fcal.end(), 
      std::get<2>(result).begin(), std::get<2>(result).end());


    result = apply_visitor(*this, std::get<2>(entry));
    X.insert(std::get<0>(result).begin(), std::get<0>(result).end());
    std::copy_if(std::get<1>(result).begin(), std::get<1>(result).end(),
      std::inserter(Fcal, Fcal.end()), Static::Functions::is_app());
    Fcal.insert(Fcal.end(), 
      std::get<2>(result).begin(), std::get<2>(result).end());
  }

  return std::make_tuple(X, FunctorList{}, Fcal);
}

DependencyFinder::result_type
DependencyFinder::operator()(const Tree::TupleExpr& e)
{
  IdentifierSet X;
  FunctorList Fcal;

  for (const auto& entry : e.pairs)
  {
    auto result = apply_visitor(*this, entry.first);
    X.insert(std::get<0>(result).begin(), std::get<0>(result).end());
    std::copy_if(std::get<1>(result).begin(), std::get<1>(result).end(),
      std::inserter(Fcal, Fcal.end()), Static::Functions::is_app());
    Fcal.insert(Fcal.end(), 
      std::get<2>(result).begin(), std::get<2>(result).end());

    result = apply_visitor(*this, entry.second);
    X.insert(std::get<0>(result).begin(), std::get<0>(result).end());
    std::copy_if(std::get<1>(result).begin(), std::get<1>(result).end(),
      std::inserter(Fcal, Fcal.end()), Static::Functions::is_app());
    Fcal.insert(Fcal.end(), 
      std::get<2>(result).begin(), std::get<2>(result).end());
  }

  return std::make_tuple(X, FunctorList{}, Fcal);
}

DependencyFinder::result_type
DependencyFinder::operator()(const Tree::AtExpr& e)
{
  IdentifierSet X;
  FunctorList F;
  FunctorList Fcal;

  auto result = apply_visitor(*this, e.lhs);
  X.insert(std::get<0>(result).begin(), std::get<0>(result).end());
  F.insert(F.end(), std::get<1>(result).begin(), std::get<1>(result).end());
  Fcal.insert(Fcal.end(), std::get<2>(result).begin(), 
    std::get<2>(result).end());

  result = apply_visitor(*this, e.rhs);
  X.insert(std::get<0>(result).begin(), std::get<0>(result).end());
  std::copy_if(std::get<1>(result).begin(), std::get<1>(result).end(),
    std::inserter(Fcal, Fcal.end()), Static::Functions::is_app());
  Fcal.insert(Fcal.end(), std::get<2>(result).begin(), 
    std::get<2>(result).end());

  return std::make_tuple(X, F, Fcal);
}

DependencyFinder::result_type
DependencyFinder::operator()(const Tree::LambdaExpr& e)
{
  IdentifierSet X;
  FunctorList F;
  FunctorList Fcal;

  for (const auto& bind : e.binds)
  {
    auto result = apply_visitor(*this, bind);
    X.insert(std::get<0>(result).begin(), std::get<0>(result).end());
    std::copy_if(std::get<1>(result).begin(), std::get<1>(result).end(),
      std::back_inserter(Fcal), Static::Functions::is_app());
    Fcal.insert(Fcal.end(), std::get<2>(result).begin(), 
      std::get<2>(result).end());
  }

  auto body = apply_visitor(*this, e.rhs);
  F.push_back(Static::Functions::CBV<IdentifierSet>
    {e.argDim, std::get<0>(body), std::get<1>(body), std::get<2>(body)});

  return std::make_tuple(X, F, Fcal);
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
  FunctorList Fcal;

  for (const auto& bind : e.binds)
  {
    auto result = apply_visitor(*this, bind);
    X.insert(std::get<0>(result).begin(), std::get<0>(result).end());
    std::copy_if(std::get<1>(result).begin(), std::get<1>(result).end(),
      std::back_inserter(Fcal), Static::Functions::is_app());
    Fcal.insert(Fcal.end(), std::get<2>(result).begin(), 
      std::get<2>(result).end());
  }

  auto body = apply_visitor(*this, e.body);
  F.push_back(Static::Functions::Base<IdentifierSet>{e.dims, 
    std::get<0>(body), std::get<1>(body), std::get<2>(body)});

  return std::make_tuple(X, F, Fcal);
}

DependencyFinder::result_type
DependencyFinder::operator()(const Tree::BangAppExpr& e)
{
  IdentifierSet X;
  FunctorList F;
  FunctorList Fcal;

  FunctorList bases;

  FunctorList F_0;
  std::vector<FunctorList> F_j;
  F_j.reserve(e.args.size());

  auto result = apply_visitor(*this, e.name);

  X.insert(std::get<0>(result).begin(), std::get<0>(result).end());
  F_0 = std::get<1>(result);
  Fcal = std::get<2>(result);

  for (const auto& expr : e.args)
  {
    result = apply_visitor(*this, expr);
    X.insert(std::get<0>(result).begin(), std::get<0>(result).end());
    F_j.push_back(std::get<1>(result));
    Fcal.insert(Fcal.end(), std::get<2>(result).begin(), 
      std::get<2>(result).end());
  }

  for (const auto& f : F_0)
  {
    auto base = get<Base>(&f);

    if (base == nullptr)
    {
      F.push_back(ApplyB{f, F_j});
    }
    else
    {
      bases.push_back(*base);
    }
  }

  result = Static::Functions::evals_applyb(bases, F_j);

  F.insert(F.end(), std::get<1>(result).begin(), std::get<1>(result).end());

  return std::make_tuple(X, F, Fcal);
}

DependencyFinder::result_type
DependencyFinder::operator()(const Tree::LambdaAppExpr& e)
{
  IdentifierSet X;
  FunctorList F;
  FunctorList Fcal;
  FunctorList cbvs;

  auto lhs = apply_visitor(*this, e.lhs);
  auto rhs = apply_visitor(*this, e.rhs);

  for (const auto& f : std::get<1>(lhs))
  {
    auto cbv = get<CBV>(&f);

    if (cbv == nullptr)
    {
      F.push_back(ApplyV{f, std::get<1>(rhs)});
    }
    else
    {
      cbvs.push_back(*cbv);
    }
  }

  auto eval_result = evals_applyv(cbvs, std::get<1>(rhs));

  X = std::get<0>(lhs);
  X.insert(std::get<0>(rhs).begin(), std::get<0>(rhs).end());
  X.insert(std::get<0>(eval_result).begin(), std::get<0>(eval_result).end());

  F.insert(F.end(), std::get<1>(eval_result).begin(), 
    std::get<1>(eval_result).end());

  Fcal = std::get<2>(lhs);
  Fcal.insert(Fcal.end(), std::get<2>(rhs).begin(), std::get<2>(rhs).end());
  Fcal.insert(Fcal.end(), std::get<2>(eval_result).begin(), 
    std::get<2>(eval_result).end());

  return std::make_tuple(X, F, Fcal);
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
  FunctorList Fcal;

  auto result = apply_visitor(*this, e.e);
  X.insert(std::get<0>(result).begin(), std::get<0>(result).end());
  F.insert(F.end(), std::get<1>(result).begin(), std::get<1>(result).end());
  Fcal.insert(Fcal.end(), std::get<2>(result).begin(), 
    std::get<2>(result).end());

  for (const auto& dim : e.dims)
  {
    result = apply_visitor(*this, dim.second);    
    X.insert(std::get<0>(result).begin(), std::get<0>(result).end());
    //F.insert(F.end(), std::get<1>(result).begin(), std::get<1>(result).end());
    std::copy_if(std::get<1>(result).begin(), std::get<1>(result).end(),
      std::back_inserter(Fcal), Static::Functions::is_app());
    Fcal.insert(Fcal.end(), std::get<2>(result).begin(), 
      std::get<2>(result).end());
  }

  return std::make_tuple(X, F, Fcal);
}

DependencyFinder::result_type
DependencyFinder::operator()(const Tree::ConditionalBestfitExpr& e)
{
  IdentifierSet X;
  FunctorList F;
  FunctorList Fcal;

  for (const auto& decl : e.declarations)
  {
    auto result = apply_visitor(*this, std::get<1>(decl));
    X.insert(std::get<0>(result).begin(), std::get<0>(result).end());
    std::copy_if(std::get<1>(result).begin(), std::get<1>(result).end(),
      std::back_inserter(Fcal), Static::Functions::is_app());
    Fcal.insert(Fcal.end(), std::get<2>(result).begin(), 
      std::get<2>(result).end());

    result = apply_visitor(*this, std::get<2>(decl));
    X.insert(std::get<0>(result).begin(), std::get<0>(result).end());
    Fcal.insert(Fcal.end(), std::get<2>(result).begin(), 
      std::get<2>(result).end());

    result = apply_visitor(*this, std::get<3>(decl));
    X.insert(std::get<0>(result).begin(), std::get<0>(result).end());
    F.insert(F.end(), std::get<1>(result).begin(), std::get<1>(result).end());
    Fcal.insert(Fcal.end(), std::get<2>(result).begin(), 
      std::get<2>(result).end());
  }

  return std::make_tuple(X, F, Fcal);
}

} //namespace Static
} //namespace TransLucid
