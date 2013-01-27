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

bool 
SemanticTransform::caching() const
{
  return m_system.cached();
}

Tree::Expr 
SemanticTransform::operator()(const Tree::ParenExpr& e)
{
  //we can drop the parens now
  return apply_visitor(*this, e.e);
}

Tree::Expr 
SemanticTransform::operator()(const Tree::LiteralExpr& e)
{
  Tree::Expr rewritten =
    Tree::LambdaAppExpr( 
      Tree::LambdaAppExpr(Tree::IdentExpr(U"construct_literal"), e.type),
      e.text);

  return apply_visitor(*this, rewritten);
}

Tree::Expr 
SemanticTransform::operator()(const Tree::UnaryOpExpr& e)
{
  //(FN1 ! (#arg0)) @ [fnname <- e.op.op, arg0 <- T(e.e)]

  Tree::Expr result = Tree::LambdaAppExpr
  (
    Tree::IdentExpr(e.op.op),
    e.e
  );

  return apply_visitor(*this, result);
}

Tree::Expr 
SemanticTransform::operator()(const Tree::BinaryOpExpr& e)
{
  //(e.op.op) . (e.lhs) . (e.rhs)
  //(FN2 ! (#arg0, #arg1))
  //  @ [fnname <- e.op.op, arg0 <- T(e.lhs), arg1 <- T(e.rhs)]

  Tree::Expr result;
    
  if (e.op.cbn)
  {
    result = Tree::PhiAppExpr
    (
      Tree::PhiAppExpr(Tree::IdentExpr(e.op.op), e.lhs),
      e.rhs
    );
  }
  else
  {
    result = Tree::LambdaAppExpr
    (
      Tree::LambdaAppExpr(Tree::IdentExpr(e.op.op), e.lhs),
      e.rhs
    );
  }

  return apply_visitor(*this, result);
}

Tree::Expr 
SemanticTransform::operator()(const Tree::BangAppExpr& e)
{
  //first check if it is #!E
  if (get<Tree::HashSymbol>(&e.name) != nullptr)
  {
    Tree::Expr rhs = apply_visitor(*this, e.args.at(0));
    return Tree::HashExpr(rhs);
  }
  else
  {
    Tree::Expr name = apply_visitor(*this, e.name);
    std::vector<Tree::Expr> args;

    for (auto expr : e.args)
    {
      args.push_back(apply_visitor(*this, expr));
    }

    return Tree::BangAppExpr(name, args);
  }
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
    //std::cerr << "adding " << renamed << " to dimensions, with index " 
    //  << *alloc << std::endl;
    m_dimscope.insert(renamed);

    //put the dim in scope too

    //we don't want to put this in scope if we're caching because now this
    //is the actual dimension and not a dimension holding onto the actual
    //dimension allocated
    if (!caching())
    {
      m_scope.push_back(*alloc);
    }

    ++alloc;
  }

  //stops us renaming more than once for multiply declared variables
  std::set<u32string> seenVars;

  //put the renamed variables in the scope
  for (const auto& evar : e.vars)
  {
    const auto& name = std::get<0>(evar);

    if (seenVars.find(name) == seenVars.end())
    {
      seenVars.insert(name);
      pushScope(name);
    }
  }

  //do the functions
  for (const auto& efun : e.funs)
  {
    const auto& name = efun.name;

    if (seenVars.find(name) == seenVars.end())
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
          getRenamed(std::get<0>(evar)),
          std::get<1>(evar),
          std::get<2>(evar),
          std::get<3>(evar)
        }
      }));
  }

  //add the functions
  for (const auto& efun : e.funs)
  {
    m_newVars.push_back(std::make_pair(currentScope, 
      Parser::FnDecl
      {
        getRenamed(efun.name),
        efun.args,
        efun.guard,
        efun.boolean,
        efun.expr
      }));
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
    const auto& renamed = getRenamed(v.first);
    m_fnScope.erase(renamed);
    m_dimscope.erase(renamed);
    popScope(v.first);
  }

  //drop the dims to hold on to
  if (!caching())
  {
    m_scope.resize(m_scope.size() - e.dims.size());
  }

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
  //std::cerr << "replacing '" << e.text << "' with '" << unique 
  //  << "'" << std::endl;

  //does this need to be replaced for a function parameter or dim
  auto iter = m_fnScope.find(unique);
  if (iter != m_fnScope.end())
  {
    //is it a dimension and are we caching?
    if (caching() && m_dimscope.find(unique) != m_dimscope.end())
    {
      //std::cerr << unique << " is a dimension" << std::endl;
      return Tree::DimensionExpr(iter->second);
    }

    //is it a call by name?
    auto cbniter = m_cbnscope.find(unique);
    if (cbniter != m_cbnscope.end())
    {
      return Tree::EvalIntenExpr(
        Tree::HashExpr(Tree::DimensionExpr(iter->second)));
    }

    return Tree::HashExpr(Tree::DimensionExpr(iter->second));
  }
  else
  {
    return Tree::IdentExpr(unique);
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

//these could be local to the one function that uses them, but at the moment,
//GCC doesn't like it
namespace
{
  struct PairCreator
  {
    template <typename A, typename B>
    auto
    operator()(A&& a, B&& b) const
    -> decltype(std::make_pair(std::forward<A>(a), std::forward<B>(b)))
    {
      return std::make_pair(std::forward<A>(a), std::forward<B>(b));
    }
  };

  struct Eraser
  {
    template <typename C, typename Value>
    void
    operator()(C&& c, Value&& v)
    {
      c.erase(v);
    }
  };
}

Tree::Expr
SemanticTransform::operator()(const Tree::BaseAbstractionExpr& e)
{
  using std::placeholders::_1;
  using std::placeholders::_2;
  //make new names, generate dimensions and put things in scope
  std::vector<dimension_index> dims;

  std::vector<u32string> renamed;

  //visit the bound dimension expressions
  std::vector<Tree::Expr> binds;
  std::transform(e.binds.begin(), e.binds.end(), std::back_inserter(binds),
    std::bind(visitor_applier(), std::ref(*this), _1)
  );

  //rename the parameters
  std::transform(e.params.begin(), e.params.end(), std::back_inserter(renamed),
    std::bind(std::mem_fn(&SemanticTransform::pushScope), this, _1));

  //generate hidden dimensions
  //but only if someone hasn't already given them to us
  if (e.dims.size() == 0)
  {
    std::generate_n(std::back_inserter(dims), e.params.size(), 
      std::bind(std::mem_fn(&System::nextHiddenDim), std::ref(m_system)));
  }
  else
  {
    dims = e.dims;
  }

  //put the hidden dimensions in scope
  std::copy(dims.begin(), dims.end(), std::back_inserter(m_scope));

  //put the hidden dimension in fnscope
  std::transform(renamed.begin(), renamed.end(), dims.begin(),
    std::inserter(m_fnScope, m_fnScope.begin()),
    std::bind(PairCreator(), _1, _2)
  );

  //visit the body
  auto body = apply_visitor(*this, e.body);

  //restore the scope
  m_scope.resize(m_scope.size() - dims.size());

  std::for_each(renamed.begin(), renamed.end(), 
    std::bind(Eraser(), m_fnScope, _1));

  Tree::BaseAbstractionExpr rewritten(binds, renamed, body);
  rewritten.dims = dims;
  rewritten.scope = m_scope;

  return rewritten;
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

  //do the binds before changing the scope
  expr.binds.clear();
  for (auto& b : e.binds)
  {
    Tree::Expr transformed = apply_visitor(*this, b);
    expr.binds.push_back(transformed);
  }

  auto unique = pushScope(e.name);

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

  expr.scope = e.scope;
  expr.scope.insert(expr.scope.end(), m_scope.begin(), m_scope.end());

  return expr;
}

Tree::Expr
SemanticTransform::operator()(const Tree::PhiExpr& e)
{
  std::vector<Tree::Expr> binds;
  for (auto& b : e.binds)
  {
    binds.push_back(apply_visitor(*this, b));
  }

  auto unique = pushScope(e.name);
  auto dim = m_system.nextHiddenDim();

  m_fnScope.insert(std::make_pair(unique, dim));
  m_cbnscope.insert(unique);
  m_scope.push_back(dim);

  auto rhs = apply_visitor(*this, e.rhs);

  m_scope.pop_back();
  m_fnScope.erase(unique);
  m_cbnscope.erase(unique);

  popScope(e.name);

  Tree::LambdaExpr lambda = Tree::LambdaExpr(unique, binds, rhs);
  lambda.argDim = dim;;
  lambda.scope = m_scope;

  return lambda;
}

Tree::Expr
SemanticTransform::operator()(const Tree::PhiAppExpr& e)
{
  Tree::Expr result = Tree::LambdaAppExpr
  (
    e.lhs, Tree::MakeIntenExpr(e.rhs)
  );

  return apply_visitor(*this, result);
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
      renames,
      m_dimscope
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
    iter = m_rename
      .insert(std::make_pair(id, decltype(m_rename)::mapped_type())).first;
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

    if (iter->second.empty())
    {
      m_rename.erase(iter);
    }
  }
}

const u32string& 
SemanticTransform::getRenamed(const u32string& name)
{
  auto iter = m_rename.find(name);

  if (iter != m_rename.end())
  {
    if (iter->second.empty())
    {
      std::cerr << "top of stack is empty" << std::endl;
    }

    return iter->second.top();
  }

  return name;
}

void
SemanticTransform::restoreScope(const ScopePtr& scope)
{
  m_cbnscope = scope->cbnParams;
  m_scope = scope->scopeDims;
  m_fnScope = scope->lookups;
  m_dimscope = scope->dimscope;

  //initialise the stack of names
  for (const auto& r : scope->renames)
  {
    auto result = 
      m_rename.insert(std::make_pair(r.first, RenameRules::mapped_type()));

    result.first->second.push(r.second);
  }
}

}
