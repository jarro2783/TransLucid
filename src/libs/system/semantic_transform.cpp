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

#include <sstream>

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
  //this is done lazily, so we don't actually visit the variables.
  //we need to visit the dimension initialisers and the expression
  //the dimension initialisers are done in the starting scope, and the 
  //expression is done with the variables and dimensions in scope
  Tree::WhereExpr w;

  //do the dimensions
  int next = 0;
  for (const auto& v : e.dims)
  {
    //get a new dimension for this dimension identifier
    auto phi_x = m_system.nextHiddenDim();

    //transform the initialiser
    w.dims.push_back(std::make_pair(v.first, apply_visitor(*this, v.second)));
    //save the new dimension
    w.dimAllocation.push_back(phi_x);

    ++next;
  }

  //put the dimensions in scope
  auto alloc = w.dimAllocation.begin();
  for (const auto& v : e.dims)
  {
    auto renamed = pushScope(v.first);
    //rename identifiers in scope
    //after visiting all the dimension initialisation expressions
    m_fnScope.insert(std::make_pair(renamed, *alloc)); 

    //put the dim in scope too
    m_scope.push_back(*alloc);

    ++alloc;
  }

  //stops us renaming more than once for multiply declared variables
  std::set<u32string> seenVars;

  //put the renamed variables in the scope
  for (const auto& evar : e.vars)
  {
    const auto& name = std::get<0>(evar);

    if (seenVars.find(name) != seenVars.end())
    {
      seenVars.insert(name);
      pushScope(name);
    }
  }

  //do the functions
  for (const auto& efun : e.funs)
  {
    const auto& name = efun.name;

    if (seenVars.find(name) != seenVars.end())
    {
      seenVars.insert(name);
      pushScope(name);
    }
  }

  //create the declarations with scope to add later
  auto currentScope = makeScope();

  //add the variables
  for (const auto& evar : e.vars)
  {
    m_newVars.push_back(std::make_pair(currentScope, 
      Parser::Variable
      {
        Parser::Equation
        {
          std::get<0>(evar),
          std::get<1>(evar),
          std::get<2>(evar),
          std::get<3>(evar)
        }
      }));
  }

  //add the functions
  for (const auto& efun : e.funs)
  {
    m_newVars.push_back(std::make_pair(currentScope, efun));
  }
  
  //visit child E
  Tree::Expr expr = apply_visitor(*this, e.e);

  //restore the scope

  //restore the variables in scope
  for (const auto& name : seenVars)
  {
    popScope(name);
  }

  //remove the dims from rewriting
  for (auto v : e.dims)
  {
    m_fnScope.erase(v.first);
  }

  //drop the dims to hold on to
  m_scope.resize(m_scope.size() - e.dims.size());

  w.e = expr;

  //we return the rewritten E and add the variables to the list of variables
  //to add to the system

  //this where clause is now effectively a wheredim clause, it will have no
  //variable identifiers because they have all been renamed and added to the
  //set of new things
  return w;
}

Tree::Expr
SemanticTransform::operator()(const Tree::IdentExpr& e)
{
  //first get the renamed name
  auto unique = getRenamed(e.text);

  //does this need to be replaced for a function parameter or dim
  auto iter = m_fnScope.find(unique);
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

  auto unique = pushScope(e.name);

  Tree::LambdaExpr expr = e;

  //1. generate a new dimension
  dimension_index argDim = expr.argDim == 0 ? 
    m_system.nextHiddenDim() : expr.argDim;

  m_fnScope.insert({unique, argDim});

  //2. store our scope dimensions and ourself
  expr.argDim = argDim;

  //3. add ourselves to the scope
  m_scope.push_back(argDim);
  //m_scopeNames.push_back(unique);

  //4. visit the child
  auto child = apply_visitor(*this, expr.rhs);

  //5. restore the scope
  m_scope.pop_back();
  //m_scope.resize(m_scope.size() - 1);
  m_fnScope.erase(unique);

  expr.name = unique;
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
  auto unique = pushScope(e.name);
  auto dim = m_system.nextHiddenDim();

  m_fnScope.insert(std::make_pair(unique, dim));
  m_cbnscope.insert(unique);

  std::vector<Tree::Expr> binds;
  for (auto& b : e.binds)
  {
    binds.push_back(apply_visitor(*this, b));
  }

  auto rhs = apply_visitor(*this, e.rhs);

  Tree::LambdaExpr lambda = Tree::LambdaExpr(unique, binds, rhs);
  lambda.argDim = e.argDim;;

  Tree::Expr le = lambda;

  Tree::Expr fun = apply_visitor(*this, le);

  m_fnScope.erase(e.name);
  m_cbnscope.erase(e.name);

  popScope(e.name);

  return fun;
}

ScopePtr
SemanticTransform::makeScope() const
{
  decltype(Scope::renames) renames;

  for (const auto& r : m_rename)
  {
    renames.insert(std::make_pair(r.first, r.second.top()));
  }

  return std::make_shared<Scope>
  (
    Scope{
      m_cbnscope, 
      m_scope,
      m_fnScope,
      renames
    }
  );
}

//open a new scope, possibly shadowing another
u32string
SemanticTransform::pushScope(const u32string& id)
{
  auto index = m_system.nextVarIndex();

  std::ostringstream os;
  os << index << "_uniqueid";

  auto unique = os.str();

  auto iter = m_rename.find(id);

  if (iter == m_rename.end())
  {
    m_rename.insert(std::make_pair(id, decltype(m_rename)::mapped_type()));
  }

  iter->second.push(u32string(unique.begin(), unique.end()));

  return iter->second.top();
}

void
SemanticTransform::popScope(const u32string& id)
{
  auto iter = m_rename.find(id);

  if (iter != m_rename.end())
  {
    iter->second.pop();
  }
}

const u32string& 
SemanticTransform::getRenamed(const u32string& name)
{
  auto iter = m_rename.find(name);

  if (iter != m_rename.end())
  {
    return iter->second.top();
  }

  return name;
}

}
