/* Replaces free variables with #hidden
   Copyright (C) 2011,2012 Jarryd Beck

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

#include <tl/free_variables.hpp>
#include <tl/system.hpp>

namespace TransLucid
{
//the free variable replacer code
//whenever this is called everything has been renamed, so there is
//no problem with name clashes

Tree::Expr
FreeVariableReplacer::replaceFree(const Tree::Expr& expr)
{
  return apply_visitor(*this, expr);
}

Tree::Expr
FreeVariableReplacer::operator()(const Tree::LambdaExpr& e)
{
  FreeVariableHelper replacer(m_system);
  replacer.addBound(m_bound);
  Tree::LambdaExpr expr = get<Tree::LambdaExpr>(replacer.replaceFree(e));

  auto& replaced = replacer.getReplaced();

  expr.free.insert(expr.free.end(), replaced.begin(), replaced.end());

  m_bound.insert(e.name);

  expr.rhs = apply_visitor(*this, expr.rhs);

  expr.argDim = e.argDim;

  m_bound.erase(e.name);

  return expr;
}

Tree::Expr
FreeVariableReplacer::operator()(const Tree::PhiExpr& e)
{
  FreeVariableHelper replacer(m_system);
  replacer.addBound(m_bound);
  Tree::PhiExpr expr = get<Tree::PhiExpr>(replacer.replaceFree(e));

  auto& replaced = replacer.getReplaced();

  expr.free.insert(expr.free.end(), replaced.begin(), replaced.end());

  m_bound.insert(e.name);

  expr.rhs = apply_visitor(*this, expr.rhs);

  m_bound.erase(e.name);

  return expr;
}

Tree::Expr 
FreeVariableHelper::operator()(const Tree::IdentExpr& e)
{
  if (m_bound.find(e.text) == m_bound.end())
  {
    dimension_index unique;
    if (m_replaced.find(e.text) == m_replaced.end())
    {
      unique = m_system.nextHiddenDim();
      m_replaced.insert(std::make_pair(e.text, unique));
    }
    else
    {
      unique = m_replaced.find(e.text)->second;
    }
    return Tree::HashExpr(Tree::DimensionExpr(unique));
  }
  else
  {
    return e;
  }
}

Tree::Expr 
FreeVariableHelper::operator()(const Tree::LambdaExpr& e)
{
  m_bound.insert(e.name);
  Tree::Expr replaced = apply_visitor(*this, e.rhs);
  m_bound.erase(e.name);

  return Tree::LambdaExpr(e.name, std::move(replaced));
}

Tree::Expr 
FreeVariableHelper::operator()(const Tree::PhiExpr& e)
{
  m_bound.insert(e.name);
  Tree::Expr replaced = apply_visitor(*this, e.rhs);
  m_bound.erase(e.name);

  return Tree::PhiExpr(e.name, std::move(replaced));
}

Tree::Expr 
FreeVariableHelper::operator()(const Tree::WhereExpr& e)
{
  //bind all of the defined variables for the E
  for (const auto& dim : e.dims)
  {
    m_bound.insert(dim.first);
  }

  for (const auto& var : e.vars)
  {
    m_bound.insert(std::get<0>(var));
  }

  Tree::Expr expr = apply_visitor(*this, e.e);
  
  Tree::WhereExpr where;

  where.e = expr;

  //replace everything in the dimension expressions
  for (const auto& dim : e.dims)
  {
    where.dims.push_back(std::make_pair(dim.first,
      apply_visitor(*this, dim.second)));
  }

  //replace everything in the var expressions
  for (const auto& var : e.vars)
  {
    where.vars.push_back(std::make_tuple(
      std::get<0>(var),
      apply_visitor(*this, std::get<1>(var)),
      apply_visitor(*this, std::get<2>(var)),
      apply_visitor(*this, std::get<3>(var))
    ));
  }

  //unbind them
  for (const auto& dim : e.dims)
  {
    m_bound.erase(dim.first);
  }

  for (const auto& var : e.vars)
  {
    m_bound.erase(std::get<0>(var));
  }

  return where;
}

Tree::Expr
FreeVariableHelper::replaceFree(const Tree::Expr& expr)
{
  m_replaced.clear();
  return apply_visitor(*this, expr);
}

}
