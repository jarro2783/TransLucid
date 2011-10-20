/* Translates AST::Expr to hyperdatons.
   Copyright (C) 2009, 2010, 2011 Jarryd Beck and John Plaice

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
  return boost::apply_visitor(*this, e);
}

WS*
WorkshopBuilder::operator()(const Tree::nil& n)
{
  return 0;
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
WorkshopBuilder::operator()(const Tree::LiteralExpr& e)
{
  //return new Workshops::TypedValueWS(m_system, e.type, e.text);
  return 0;
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
  return boost::apply_visitor(*this, e.e);
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
  return 0;
}

WS*
WorkshopBuilder::operator()(const Tree::BangAppExpr& e)
{
  WS* name = boost::apply_visitor(*this, e.name);
  std::vector<WS*> args;

  for (auto& expr : e.args)
  {
    args.push_back(boost::apply_visitor(*this, expr));
  }

  return new Workshops::BangOpWS(*m_system, name, args);
}

WS*
WorkshopBuilder::operator()(const Tree::IfExpr& e)
{
  WS* condition = boost::apply_visitor(*this, e.condition);
  WS* then = boost::apply_visitor(*this, e.then);
  WS* else_ = boost::apply_visitor(*this, e.else_);

  std::vector<std::pair<WS*, WS*>> else_ifs;

  for(auto& v : e.else_ifs)
  {
    else_ifs.push_back(std::make_pair
    (
      boost::apply_visitor(*this, v.first),
      boost::apply_visitor(*this, v.second)
    ));
  }

  return new Workshops::IfWS(condition, then, else_ifs, else_);
}

WS*
WorkshopBuilder::operator()(const Tree::HashExpr& e)
{
  WS* expr = boost::apply_visitor(*this, e.e);
  return new Workshops::HashWS(*m_system, expr);
}

WS*
WorkshopBuilder::operator()(const Tree::TupleExpr& e)
{
  std::list<std::pair<WS*, WS*>> elements;
  for(auto& v : e.pairs)
  {
    WS* lhs = boost::apply_visitor(*this, v.first);
    WS* rhs = boost::apply_visitor(*this, v.second);
    elements.push_back(std::make_pair(lhs, rhs));
  }
  return new Workshops::TupleWS(*m_system, elements);
}

WS*
WorkshopBuilder::operator()(const Tree::AtExpr& e)
{
  WS* lhs = boost::apply_visitor(*this, e.lhs);
  WS* rhs = boost::apply_visitor(*this, e.rhs);

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
WorkshopBuilder::operator()(const Tree::BangExpr& e)
{
  WS* rhs = boost::apply_visitor(*this, e.rhs);

  return new Workshops::BaseAbstractionWS
  (
    e.name,
    e.argDim,
    e.scope,
    rhs
  );
}

WS*
WorkshopBuilder::operator()(const Tree::LambdaExpr& e)
{
  WS* rhs = boost::apply_visitor(*this, e.rhs);

  return new Workshops::LambdaAbstractionWS
  (
    e.name,
    e.argDim,
    e.scope,
    rhs
  );
}

WS*
WorkshopBuilder::operator()(const Tree::PhiExpr& e)
{
  WS* rhs = boost::apply_visitor(*this, e.rhs);

  return new Workshops::NamedAbstractionWS
  (
    e.name,
    e.argDim,
    e.odometerDim,
    e.scope,
    rhs
  );
}

WS* 
WorkshopBuilder::operator()(const Tree::LambdaAppExpr& e)
{
  //create a LambdaApplicationWS with the compiled sub expression
  WS* lhs = boost::apply_visitor(*this, e.lhs);
  WS* rhs = boost::apply_visitor(*this, e.rhs);
  return new Workshops::LambdaApplicationWS(lhs, rhs);
}

WS* 
WorkshopBuilder::operator()(const Tree::PhiAppExpr& e)
{
  WS* lhs = boost::apply_visitor(*this, e.lhs);
  WS* rhs = boost::apply_visitor(*this, e.rhs);
  return new Workshops::NameApplicationWS(lhs, rhs);
}

WS* 
WorkshopBuilder::operator()(const Tree::WhereExpr& e)
{
  //the where expression must already be annotated and transformed
  //which means that we can simply translate the expression

  return boost::apply_visitor(*this, e.e);
}

} //namespace TransLucid
