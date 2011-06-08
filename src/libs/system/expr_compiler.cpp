/* Translates AST::Expr to hyperdatons.
   Copyright (C) 2009, 2010 Jarryd Beck and John Plaice

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

#include <tl/compiled_functors.hpp>
#include <tl/consthd.hpp>
#include <tl/expr_compiler.hpp>
#include <tl/fixed_indexes.hpp>
#include <tl/rename.hpp>
#include <tl/utility.hpp>

#include <boost/variant.hpp>

#include <sstream>

namespace TransLucid
{

ExprCompiler::ExprCompiler(System* system)
: m_system(system)
{
}

ExprCompiler::~ExprCompiler()
{
}

WS*
ExprCompiler::compile_for_equation(const Tree::Expr& e)
{
  return boost::apply_visitor(*this, e);
}

WS*
ExprCompiler::compile_top_level(const Tree::Expr& e)
{
  WS* h = boost::apply_visitor(*this, e);

  if (h == 0)
  {
    return 0;
  }
  else
  {
    //return new Hyperdatons::SystemEvaluationWS(m_system, h);
  }
}

WS*
ExprCompiler::operator()(const Tree::nil& n)
{
  return 0;
}

WS*
ExprCompiler::operator()(bool b)
{
  return new Hyperdatons::BoolConstWS(b);
}

WS*
ExprCompiler::operator()(Special s)
{
  return new Hyperdatons::SpecialConstWS(s);
}

WS*
ExprCompiler::operator()(const mpz_class& i)
{
  return new Hyperdatons::IntmpConstWS(i);
}

WS*
ExprCompiler::operator()(char32_t c)
{
  return new Hyperdatons::UCharConstWS(c);
}

WS*
ExprCompiler::operator()(const u32string& s)
{
  return new Hyperdatons::UStringConstWS(s);
}

WS*
ExprCompiler::operator()(const Tree::LiteralExpr& e)
{
  //return new Hyperdatons::TypedValueWS(m_system, e.type, e.text);
  return 0;
}

WS*
ExprCompiler::operator()(const Tree::DimensionExpr& e)
{
  return new Hyperdatons::DimensionWS(*m_system, e.text);
}

WS*
ExprCompiler::operator()(const Tree::IdentExpr& e)
{
  return new Hyperdatons::IdentWS(m_system, e.text);
}

WS* 
ExprCompiler::operator()(const Tree::ParenExpr& e)
{
  return boost::apply_visitor(*this, e.e);
}

WS*
ExprCompiler::operator()(const Tree::UnaryOpExpr& e)
{
  WS* operand = boost::apply_visitor(*this, e.e);
  return new Hyperdatons::UnaryOpWS(m_system, e.op.op, operand);
}

WS*
ExprCompiler::operator()(const Tree::BinaryOpExpr& e)
{
  #if 0
  WS* lhs = boost::apply_visitor(*this, e.lhs);
  WS* rhs = boost::apply_visitor(*this, e.rhs);

  return new Hyperdatons::BinaryOpWS(m_system, {lhs, rhs}, e.op.op);
  #endif
  throw "ExprCompiler::operator()(BinaryOpExpr)";
  return 0;
}

WS*
ExprCompiler::operator()(const Tree::IfExpr& e)
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

  return new Hyperdatons::IfWS(condition, then, else_ifs, else_);
}

WS*
ExprCompiler::operator()(const Tree::HashExpr& e)
{
  WS* expr = boost::apply_visitor(*this, e.e);
  return new Hyperdatons::HashWS(*m_system, expr);
}

WS*
ExprCompiler::operator()(const Tree::TupleExpr& e)
{
  std::list<std::pair<WS*, WS*>> elements;
  for(auto& v : e.pairs)
  {
    WS* lhs = boost::apply_visitor(*this, v.first);
    WS* rhs = boost::apply_visitor(*this, v.second);
    elements.push_back(std::make_pair(lhs, rhs));
  }
  return new Hyperdatons::TupleWS(*m_system, elements);
}

WS*
ExprCompiler::operator()(const Tree::AtExpr& e)
{
  WS* lhs = boost::apply_visitor(*this, e.lhs);
  WS* rhs = boost::apply_visitor(*this, e.rhs);

  return new Hyperdatons::AtWS(lhs, rhs);
}

WS*
ExprCompiler::operator()(const Tree::LambdaExpr& e)
{
//TODO: we will get to this eventually
#if 0
  //generate a new dimension
  tuple_t k = {{DIM_ID, generate_string(U"_uniquedim")}};
  Intmp uniqueIndex = (*m_system)(Tuple(k)).first.value<Intmp>();
  dimension_index index = uniqueIndex.value().get_ui();

  //generate a unique name alpha
  k[DIM_ID] = generate_string(U"_unique");
  Intmp nameNum = (*m_system)(Tuple(k)).first.value<Intmp>();
  std::ostringstream os;
  os << nameNum.value() << "_lambdaparam";
  u32string uniqueName = to_u32string(os.str());

  //rename name to alpha in the sub expression
  Tree::Expr renamed = RenameIdentifier(e.name, uniqueName).rename(e.rhs);

  //add alpha = #_uniquedim to the system
  WS* hashUnique = new HashIndexWS(index);
  tuple_t addContext = {{DIM_ID, generate_string(uniqueName)}};
  //m_system->addExpr(Tuple(addContext), hashUnique);
  m_system->addEquation(uniqueName, hashUnique);

  //make a LambdaAbstractionWS
  WS* rhs = boost::apply_visitor(*this, renamed);
  return new Hyperdatons::LambdaAbstractionWS(m_system, e.name, index, rhs);
#endif
  return 0;
}

WS*
ExprCompiler::operator()(const Tree::PhiExpr& e)
{
  //generate a new dimension gamma

  //generate a unique name alpha
  
  //tag e with j (just &e)

  //add alpha | hd(#gamma) == j = e.rhs @ [gamma : tl(#gamma)]

  //create a PhiAbstractionWS
  return 0;
}

WS* 
ExprCompiler::operator()(const Tree::ValueAppExpr& e)
{
  //create a LambdaApplicationWS with the compiled sub expression
  WS* lhs = boost::apply_visitor(*this, e.lhs);
  WS* rhs = boost::apply_visitor(*this, e.rhs);
  return new Hyperdatons::LambdaApplicationWS(m_system, lhs, rhs);
}

WS* 
ExprCompiler::operator()(const Tree::NameAppExpr& e)
{
  return 0;
}

} //namespace TransLucid
