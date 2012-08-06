/* Rewrites trees according to the semantics transformation
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

#include <tl/output.hpp>
#include <tl/semantic_transform.hpp>
#include <tl/system.hpp>
#include <tl/utility.hpp>

// There are several problems in writing this transformation:
//   1. The evaluator needs to be lazy.
//   2. Everything needs to be renamed.
//   3. The scope at each where clause needs to be known to enable laziness.
//   4. There are several different ways to rewrite an identifier depending
//       on what it was.
//
// The solution is therefore this: Rename everything on the fly, and use the
// unique names for determining what to rewrite expressions to. Then, hold
// on to the scope for each where clause.
// The scope is stored as a shared pointer to a scope object, because every
// entity declared inside a where clause will have the same scope.

namespace TransLucid
{

Tree::Expr
SemanticTransform::transform(const Tree::Expr& e)
{
  return apply_visitor(*this, e);
}

Tree::Expr
SemanticTransform::operator()(const Tree::WhereExpr& e)
{
  //all the names are already unique
  Tree::WhereExpr w;

  std::vector<dimension_index> myLin;
  
  //generate new label
  mpz_class label = w.myLabel != 0 ? w.myLabel : m_system.nextWhere();

  w.myLabel = label;

  //do the dimensions
  int next = 0;
  for (const auto& v : e.dims)
  {
    //get a new dimension for this dimension identifier
    auto theta = m_system.nextHiddenDim();

    //transform the initialiser
    w.dims.push_back(std::make_pair(v.first, apply_visitor(*this, v.second)));
    //save the new dimension
    w.dimAllocation.push_back(theta);

    ++next;
  }

  auto alloc = w.dimAllocation.begin();
  for (const auto& v : e.dims)
  {
    //rename identifiers in scope
    //after visiting all the dimension initialisation expressions
    m_fnScope.insert(std::make_pair(v.first, *alloc)); 

    //put the dim in scope too
    m_scope.push_back(*alloc);

    ++alloc;
  }


  //do the variables
  for (const auto& evar : e.vars)
  {
    //expr
    Tree::Expr varExpr = apply_visitor(*this, std::get<3>(evar));

    //guard
    Tree::Expr guardExpr = apply_visitor(*this, std::get<1>(evar));

    //boolean
    Tree::Expr booleanExpr = apply_visitor(*this, std::get<2>(evar));;

    auto newEqn = Parser::Equation
      (
        std::get<0>(evar),
        guardExpr,
        booleanExpr,
        varExpr
      );

    m_newVars.push_back
    (
      newEqn
    );

    w.vars.push_back(newEqn);
  }

  //do the functions
  for (const auto& efun : e.funs)
  {
  }
  
  //visit child E
  Tree::Expr expr = apply_visitor(*this, e.e);

  //restore the scope
  for (auto v : e.dims)
  {
    m_fnScope.erase(v.first);
  }

  m_scope.resize(m_scope.size() - e.dims.size());

  w.e = expr;

  //we return the rewritten E and add the variables to the list of variables
  //to add to the system

  return w;
}

Tree::Expr
SemanticTransform::operator()(const Tree::IdentExpr& e)
{
  //does this need to be replaced for a function parameter or dim
  auto iter = m_fnScope.find(e.text);
  if (iter != m_fnScope.end())
  {
    //is it a call by name?
    auto cbniter = m_cbnscope.find(e.text);
    if (cbniter != m_cbnscope.end())
    {
      return Tree::EvalIntenExpr(
        Tree::HashExpr(Tree::DimensionExpr(iter->second)));
    }

    return Tree::HashExpr(Tree::DimensionExpr(iter->second));
  }
  else
  {
    return e;
  }
}

Tree::Expr
SemanticTransform::operator()(const Tree::MakeIntenExpr& e)
{
  Tree::MakeIntenExpr inten;
  inten.expr = apply_visitor(*this, e.expr);

  inten.scope.insert(inten.scope.end(), m_scope.begin(), m_scope.end());

  return inten;
}

Tree::Expr
SemanticTransform::operator()(const Tree::LambdaExpr& e)
{
  //1. generate a new dimension
  //2. store our scope dimensions
  //3. add ourselves to the scope
  //4. visit the child
  //5. restore the scope

  Tree::LambdaExpr expr = e;

  //1. generate a new dimension
  dimension_index argDim = expr.argDim == 0 ? 
    m_system.nextHiddenDim() : expr.argDim;

  m_fnScope.insert({e.name, argDim});

  //2. store our scope dimensions and ourself
  expr.argDim = argDim;

  //3. add ourselves to the scope
  m_scope.push_back(argDim);
  m_scopeNames.push_back(e.name);

  //4. visit the child
  auto child = apply_visitor(*this, expr.rhs);

  //5. restore the scope
  m_scope.pop_back();
  //m_scope.resize(m_scope.size() - 1);
  m_fnScope.erase(e.name);

  expr.rhs = child;

  expr.binds.clear();
  for (auto& b : e.binds)
  {
    Tree::Expr transformed = apply_visitor(*this, b);
    expr.binds.push_back(transformed);
  }

  expr.scope = e.scope;
  expr.scope.insert(expr.scope.end(), m_scope.begin(), m_scope.end());

  return expr;
}

Tree::Expr
SemanticTransform::operator()(const Tree::PhiExpr& e)
{
  auto dim = m_system.nextHiddenDim();

  //if this shadows

  m_fnScope.insert(std::make_pair(e.name, dim));
  m_cbnscope.insert(e.name);

  std::vector<Tree::Expr> binds;
  for (auto& b : e.binds)
  {
    binds.push_back(apply_visitor(*this, b));
  }

  Tree::LambdaExpr lambda = Tree::LambdaExpr(e.name, binds, e.rhs);
  lambda.argDim = e.argDim;;

  Tree::Expr le = lambda;

  Tree::Expr fun = apply_visitor(*this, le);

  m_fnScope.erase(e.name);
  m_cbnscope.erase(e.name);

  return fun;
}

ScopePtr
SemanticTransform::makeScope() const
{
}

}
