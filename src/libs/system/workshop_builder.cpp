/* Translates AST::Expr to hyperdatons.
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

/**
 * @file expr_compiler.cpp
 * Turns expressions into workshops.
 */

#include <tl/output.hpp>

#include <tl/ast.hpp>
#include <tl/constws.hpp>
#include <tl/eval_workshops.hpp>
#include <tl/workshop_builder.hpp>
#include <tl/fixed_indexes.hpp>
#include <tl/rename.hpp>
#include <tl/utility.hpp>

namespace TransLucid
{

WorkshopBuilder::WorkshopBuilder(System* system)
: m_system(system)
{
}

WorkshopBuilder::~WorkshopBuilder()
{
}

WS*
WorkshopBuilder::build_workshops(const Tree::Expr& e)
{
  return apply_visitor(*this, e);
}

WS*
WorkshopBuilder::operator()(const Tree::nil& n)
{
  return nullptr;
}

WS*
WorkshopBuilder::operator()(bool b)
{
  return new Workshops::BoolConstWS(b);
}

WS*
WorkshopBuilder::operator()(Special s)
{
  return new Workshops::SpecialConstWS(s);
}

WS*
WorkshopBuilder::operator()(const mpz_class& i)
{
  return new Workshops::IntmpConstWS(i);
}

WS*
WorkshopBuilder::operator()(char32_t c)
{
  return new Workshops::UCharConstWS(c);
}

WS*
WorkshopBuilder::operator()(const u32string& s)
{
  return new Workshops::UStringConstWS(s);
}

WS*
WorkshopBuilder::operator()(const Tree::HashSymbol& s)
{
  return new Workshops::HashSymbolWS();
}

WS* 
WorkshopBuilder::operator()(const Tree::HostOpExpr& e)
{
  return new Workshops::HostOpWS(*m_system, e.name);
}

WS*
WorkshopBuilder::operator()(const Tree::LiteralExpr& e)
{
  //return new Workshops::TypedValueWS(m_system, e.type, e.text);
  std::cerr << "warning: compiling literal node" << std::endl;
  return nullptr;
}

WS*
WorkshopBuilder::operator()(const Tree::DimensionExpr& e)
{
  if (e.text.empty())
  {
    return new Workshops::DimensionWS(*m_system, e.dim);
  }
  else
  {
    return new Workshops::DimensionWS(*m_system, e.text);
  }
}

WS*
WorkshopBuilder::operator()(const Tree::IdentExpr& e)
{
  return new Workshops::IdentWS(m_system->lookupIdentifiers(), e.text);
}

WS* 
WorkshopBuilder::operator()(const Tree::ParenExpr& e)
{
  return apply_visitor(*this, e.e);
}

WS*
WorkshopBuilder::operator()(const Tree::UnaryOpExpr& e)
{
  //this should be compiled out
  throw "WorkshopBuilder::operator()(UnaryOpExpr)";
}

WS*
WorkshopBuilder::operator()(const Tree::BinaryOpExpr& e)
{
  //this should be compiled out
  throw "WorkshopBuilder::operator()(BinaryOpExpr)";
  return nullptr;
}

WS*
WorkshopBuilder::operator()(const Tree::BangAppExpr& e)
{
  WS* name = apply_visitor(*this, e.name);
  std::vector<WS*> args;

  for (auto& expr : e.args)
  {
    args.push_back(apply_visitor(*this, expr));
  }

  return new Workshops::BangOpWS(*m_system, name, args);
}

WS*
WorkshopBuilder::operator()(const Tree::IfExpr& e)
{
  WS* condition = apply_visitor(*this, e.condition);
  WS* then = apply_visitor(*this, e.then);
  WS* else_ = apply_visitor(*this, e.else_);

  std::vector<std::pair<WS*, WS*>> else_ifs;

  for(auto& v : e.else_ifs)
  {
    else_ifs.push_back(std::make_pair
    (
      apply_visitor(*this, v.first),
      apply_visitor(*this, v.second)
    ));
  }

  return new Workshops::IfWS(condition, then, else_ifs, else_);
}

WS* 
WorkshopBuilder::operator()(const Tree::MakeIntenExpr& e)
{
  std::vector<WS*> binds;

  for (auto& b : e.binds)
  {
    binds.push_back(apply_visitor(*this, b));
  }

  WS* rhs = apply_visitor(*this, e.expr);
  WS* result = new Workshops::MakeIntenWS(*m_system, rhs, binds, e.scope);

  return result;
}

WS* 
WorkshopBuilder::operator()(const Tree::EvalIntenExpr& e)
{
  WS* rhs = apply_visitor(*this, e.expr);
  WS* result = new Workshops::EvalIntenWS(rhs);

  return result;
}

WS*
WorkshopBuilder::operator()(const Tree::HashExpr& e)
{
  WS* expr = apply_visitor(*this, e.e);
  return new Workshops::HashWS(*m_system, expr, e.cached);
}

WS*
WorkshopBuilder::operator()(const Tree::RegionExpr& e)
{
  Workshops::RegionWS::EntryWorkshops entries;
  for (auto& v : e.entries)
  {
    WS* lhs = apply_visitor(*this, std::get<0>(v));
    WS* rhs = apply_visitor(*this, std::get<2>(v));
    entries.push_back(std::make_tuple(lhs, std::get<1>(v), rhs));
  }

  return new Workshops::RegionWS(*m_system, entries);
}

WS*
WorkshopBuilder::operator()(const Tree::TupleExpr& e)
{
  std::list<std::pair<WS*, WS*>> elements;
  for(auto& v : e.pairs)
  {
    WS* lhs = apply_visitor(*this, v.first);
    WS* rhs = apply_visitor(*this, v.second);
    elements.push_back(std::make_pair(lhs, rhs));
  }
  return new Workshops::TupleWS(*m_system, elements);
}

WS*
WorkshopBuilder::operator()(const Tree::AtExpr& e)
{
  WS* lhs = apply_visitor(*this, e.lhs);
  WS* rhs = apply_visitor(*this, e.rhs);

  //if the rhs is a tuple, then we can do better
  Workshops::TupleWS* tuplerhs = dynamic_cast<Workshops::TupleWS*>(rhs);
  if (tuplerhs != nullptr)
  {
    WS* result = new 
      Workshops::AtTupleWS(lhs, tuplerhs->getElements(), *m_system);
    tuplerhs->releaseElements();
    delete tuplerhs;

    return result;
  }
  else
  {
    return new Workshops::AtWS(lhs, rhs);
  }
}

WS* 
WorkshopBuilder::operator()(const Tree::BaseAbstractionExpr& e)
{
  std::vector<WS*> binds;

  for (auto& b : e.binds)
  {
    binds.push_back(apply_visitor(*this, b));
  }

  WS* body = apply_visitor(*this, e.body);

  return new Workshops::BaseAbstractionWS(m_system, e.dims, 
    e.scope, binds, body);
}

WS*
WorkshopBuilder::operator()(const Tree::LambdaExpr& e)
{
  WS* rhs = apply_visitor(*this, e.rhs);

  std::vector<WS*> binds;

  for (auto& b : e.binds)
  {
    binds.push_back(apply_visitor(*this, b));
  }

  return new Workshops::LambdaAbstractionWS
  (
    m_system,
    e.name,
    e.argDim,
    rhs,
    binds,
    e.scope
  );
}

WS*
WorkshopBuilder::operator()(const Tree::PhiExpr& e)
{
  throw "error: WorkshopBuilder(PhiExpr) reached";
}

WS* 
WorkshopBuilder::operator()(const Tree::LambdaAppExpr& e)
{
  //create a LambdaApplicationWS with the compiled sub expression
  WS* lhs = apply_visitor(*this, e.lhs);
  WS* rhs = apply_visitor(*this, e.rhs);
  return new Workshops::LambdaApplicationWS(lhs, rhs);
}

WS* 
WorkshopBuilder::operator()(const Tree::PhiAppExpr& e)
{
  throw "WorkshopBuilder::operator()(PhiAppExpr)";
}

WS* 
WorkshopBuilder::operator()(const Tree::WhereExpr& e)
{
  std::vector<std::pair<int, WS*>> dims;

  auto alloc = e.dimAllocation.begin();
  for (auto v : e.dims)
  {
    dims.push_back(std::make_pair(*alloc, apply_visitor(*this, v.second)));
    ++alloc;
  }

  auto expr = apply_visitor(*this, e.e);

  return new Workshops::WhereWS(expr, dims, *m_system);
}

WS* 
WorkshopBuilder::operator()(const Tree::ConditionalBestfitExpr& e)
{
  std::vector<CompiledEquationWS> compiled;
  for (auto equation : e.declarations)
  {
    auto guard = apply_visitor(*this, std::get<1>(equation));
    auto boolean = apply_visitor(*this, std::get<2>(equation));
    auto expr = apply_visitor(*this, std::get<3>(equation));

    compiled.push_back
    (
      CompiledEquationWS
      (
        EquationGuard
        (
          guard, boolean
        ),
        std::shared_ptr<WS>(expr),
        std::get<0>(equation)
      )
    );
  }

  auto bestfit = std::unique_ptr<ConditionalBestfitWS>(
    new ConditionalBestfitWS(compiled));

  return bestfit.release();
}

} //namespace TransLucid
