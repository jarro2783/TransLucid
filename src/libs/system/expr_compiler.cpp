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

ExprCompiler::ExprCompiler(SystemHD* system)
: m_system(system)
{
}

ExprCompiler::~ExprCompiler()
{
}

HD*
ExprCompiler::compile_for_equation(const Tree::Expr& e)
{
  return boost::apply_visitor(*this, e);
}

HD*
ExprCompiler::compile_top_level(const Tree::Expr& e)
{
  HD* h = boost::apply_visitor(*this, e);

  if (h == 0)
  {
    return 0;
  }
  else
  {
    return new Hyperdatons::SystemEvaluationHD(m_system, h);
  }
}

HD*
ExprCompiler::operator()(const Tree::nil& n)
{
  return 0;
}

HD*
ExprCompiler::operator()(bool b)
{
  return new Hyperdatons::BoolConstHD(b);
}

HD*
ExprCompiler::operator()(Special::Value s)
{
  return new Hyperdatons::SpecialConstHD(s);
}

HD*
ExprCompiler::operator()(const mpz_class& i)
{
  return new Hyperdatons::IntmpConstHD(i);
}

HD*
ExprCompiler::operator()(char32_t c)
{
  return new Hyperdatons::UCharConstHD(c);
}

HD*
ExprCompiler::operator()(const u32string& s)
{
  return new Hyperdatons::UStringConstHD(s);
}

HD*
ExprCompiler::operator()(const Tree::ConstantExpr& e)
{
  return new Hyperdatons::TypedValueHD(m_system, e.type, e.text);
}

HD*
ExprCompiler::operator()(const Tree::DimensionExpr& e)
{
  return new Hyperdatons::DimensionHD(m_system, e.text);
}

HD*
ExprCompiler::operator()(const Tree::IdentExpr& e)
{
  return new Hyperdatons::IdentHD(m_system, e.text);
}

HD* 
ExprCompiler::operator()(const Tree::ParenExpr& e)
{
  return boost::apply_visitor(*this, e.e);
}

HD*
ExprCompiler::operator()(const Tree::UnaryOpExpr& e)
{
  HD* operand = boost::apply_visitor(*this, e.e);
  return new Hyperdatons::UnaryOpHD(m_system, e.op.op, operand);
}

HD*
ExprCompiler::operator()(const Tree::BinaryOpExpr& e)
{
  HD* lhs = boost::apply_visitor(*this, e.lhs);
  HD* rhs = boost::apply_visitor(*this, e.rhs);

  return new Hyperdatons::BinaryOpHD(m_system, {lhs, rhs}, e.op.op);
}

HD*
ExprCompiler::operator()(const Tree::IfExpr& e)
{
  HD* condition = boost::apply_visitor(*this, e.condition);
  HD* then = boost::apply_visitor(*this, e.then);
  HD* else_ = boost::apply_visitor(*this, e.else_);

  std::vector<std::pair<HD*, HD*>> else_ifs;

  for(auto& v : e.else_ifs)
  {
    else_ifs.push_back(std::make_pair
    (
      boost::apply_visitor(*this, v.first),
      boost::apply_visitor(*this, v.second)
    ));
  }

  return new Hyperdatons::IfHD(condition, then, else_ifs, else_);
}

HD*
ExprCompiler::operator()(const Tree::HashExpr& e)
{
  HD* expr = boost::apply_visitor(*this, e.e);
  return new Hyperdatons::HashHD(m_system, expr);
}

HD*
ExprCompiler::operator()(const Tree::TupleExpr& e)
{
  std::list<std::pair<HD*, HD*>> elements;
  for(auto& v : e.pairs)
  {
    HD* lhs = boost::apply_visitor(*this, v.first);
    HD* rhs = boost::apply_visitor(*this, v.second);
    elements.push_back(std::make_pair(lhs, rhs));
  }
  return new Hyperdatons::TupleHD(m_system, elements);
}

HD*
ExprCompiler::operator()(const Tree::AtExpr& e)
{
  HD* lhs = boost::apply_visitor(*this, e.lhs);
  HD* rhs = boost::apply_visitor(*this, e.rhs);

  if (e.absolute)
  {
    return new Hyperdatons::AtAbsoluteHD(lhs, rhs);
  }
  else
  {
    return new Hyperdatons::AtRelativeHD(lhs, rhs);
  }
}

HD*
ExprCompiler::operator()(const Tree::LambdaExpr& e)
{
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
  HD* hashUnique = new HashIndexHD(index);
  tuple_t addContext = {{DIM_ID, generate_string(uniqueName)}};
  //m_system->addExpr(Tuple(addContext), hashUnique);
  m_system->addEquation(uniqueName, hashUnique);

  //make a LambdaAbstractionHD
  HD* rhs = boost::apply_visitor(*this, renamed);
  return new Hyperdatons::LambdaAbstractionHD(m_system, e.name, index, rhs);
}

HD*
ExprCompiler::operator()(const Tree::PhiExpr& e)
{
  //generate a new dimension gamma

  //generate a unique name alpha
  
  //tag e with j (just &e)

  //add alpha | hd(#gamma) == j = e.rhs @ [gamma : tl(#gamma)]

  //create a PhiAbstractionHD
  return 0;
}

HD* 
ExprCompiler::operator()(const Tree::ValueAppExpr& e)
{
  //create a LambdaApplicationHD with the compiled sub expression
  HD* lhs = boost::apply_visitor(*this, e.lhs);
  HD* rhs = boost::apply_visitor(*this, e.rhs);
  return new Hyperdatons::LambdaApplicationHD(m_system, lhs, rhs);
}

HD* 
ExprCompiler::operator()(const Tree::NameAppExpr& e)
{
  return 0;
}

} //namespace TransLucid
