/* TODO: Give a descriptor.
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

#include <tl/expr_compiler.hpp>
#include <tl/compiled_functors.hpp>
#include <boost/variant.hpp>

namespace TransLucid
{

using boost::fusion::at_c;

ExprCompiler::ExprCompiler(HD* i)
: m_i(i)
{
}

ExprCompiler::~ExprCompiler()
{
}

HD*
ExprCompiler::compile(const Tree::Expr& e)
{
  return boost::apply_visitor(*this, e);
}

HD*
ExprCompiler::operator()(const Tree::nil& n)
{
  return 0;
}

HD*
ExprCompiler::operator()(bool b)
{
  return new CompiledFunctors::BoolConst(b);
}

HD*
ExprCompiler::operator()(Special::Value s)
{
  return new CompiledFunctors::SpecialConst(s);
}

HD*
ExprCompiler::operator()(const mpz_class& i)
{
  return new CompiledFunctors::Integer(m_i, i);
}

HD*
ExprCompiler::operator()(char32_t c)
{
  return new CompiledFunctors::UcharConst(c);
}

HD*
ExprCompiler::operator()(const u32string& s)
{
  return new CompiledFunctors::StringConst(s);
}

HD*
ExprCompiler::operator()(const Tree::ConstantExpr& e)
{
  return new CompiledFunctors::Constant(m_i, e.type, e.text);
}

HD*
ExprCompiler::operator()(const Tree::DimensionExpr& e)
{
  return new CompiledFunctors::Dimension(m_i, e.text);
}

HD*
ExprCompiler::operator()(const Tree::IdentExpr& e)
{
  return new CompiledFunctors::Ident(m_i, e.text);
}

HD*
ExprCompiler::operator()(const Tree::UnaryOpExpr& e)
{
  HD* operand = boost::apply_visitor(*this, e.e);
  return new CompiledFunctors::UnaryOp(e.op, operand);
}

HD*
ExprCompiler::operator()(const Tree::BinaryOpExpr& e)
{
  HD* lhs = boost::apply_visitor(*this, e.lhs);
  HD* rhs = boost::apply_visitor(*this, e.rhs);

  return new CompiledFunctors::BinaryOp(m_i, {lhs, rhs}, e.op.op);
}

HD*
ExprCompiler::operator()(const Tree::IfExpr& e)
{
  HD* condition = boost::apply_visitor(*this, e.condition);
  HD* then = boost::apply_visitor(*this, e.then);
  HD* else_ = boost::apply_visitor(*this, e.else_);

  std::vector<std::pair<HD*, HD*>> else_ifs;

  BOOST_FOREACH(auto& v, e.else_ifs)
  {
    else_ifs.push_back(std::make_pair
    (
      boost::apply_visitor(*this, v.first),
      boost::apply_visitor(*this, v.second)
    ));
  }

  return new CompiledFunctors::If(condition, then, else_ifs, else_);
}

HD*
ExprCompiler::operator()(const Tree::HashExpr& e)
{
  HD* expr = boost::apply_visitor(*this, e.e);
  return new CompiledFunctors::Hash(m_i, expr);
}

HD*
ExprCompiler::operator()(const Tree::BuildTupleExpr& e)
{
  std::list<std::pair<HD*, HD*>> elements;
  BOOST_FOREACH(auto& v, e.pairs)
  {
    HD* lhs = boost::apply_visitor(*this, at_c<0>(v));
    HD* rhs = boost::apply_visitor(*this, at_c<1>(v));
    elements.push_back(std::make_pair(lhs, rhs));
  }
  return new CompiledFunctors::BuildTuple(m_i, elements);
}

HD*
ExprCompiler::operator()(const Tree::AtExpr& e)
{
  HD* lhs = boost::apply_visitor(*this, e.lhs);
  HD* rhs = boost::apply_visitor(*this, e.rhs);

  if (e.relative)
  {
    return new CompiledFunctors::AtRelative(lhs, rhs);
  }
  else
  {
    return new CompiledFunctors::AtAbsolute(lhs, rhs);
  }
}

} //namespace TransLucid
