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

#include <tl/semantic_transform.hpp>
#include <tl/system.hpp>
#include <tl/utility.hpp>

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
  Tree::WhereExpr w = e;

  std::vector<dimension_index> myLin;
  
  //generate new label
  dimension_index label = m_system.nextHiddenDim();
  w.myDim = label;

  //store L_out
  //visit children variables
  m_Lout.push_back(label);

  //L_in is all of the inner where clauses 
  //apply_visitor(*this, e.e);

  m_Lin.clear();
  for (const auto& evar : e.vars)
  {
    //expr
    m_Lin.clear();
    Tree::Expr varExpr = apply_visitor(*this, std::get<3>(evar));
    myLin.insert(myLin.end(), m_Lin.begin(), m_Lin.end());

    //guard
    m_Lin.clear();
    Tree::Expr guardExpr = apply_visitor(*this, std::get<1>(evar));
    myLin.insert(myLin.end(), m_Lin.begin(), m_Lin.end());

    //boolean
    m_Lin.clear();
    Tree::Expr booleanExpr = apply_visitor(*this, std::get<2>(evar));;
    myLin.insert(myLin.end(), m_Lin.begin(), m_Lin.end());

    m_newVars.push_back
    (
      Parser::Equation
      (
        std::get<0>(evar),
        guardExpr,
        booleanExpr,
        varExpr
      )
    );
    m_Lin.clear();
  }

  //visit child E
  Tree::Expr expr = apply_visitor(*this, e.e);
  myLin.insert(myLin.end(), m_Lin.begin(), m_Lin.end());

  //we have now fully computed the where clause's L_in
  w.Lin = myLin;

  //rewrite E to
  //E @ [d_i <- E_i, Lin_i <- 0] @ [l <- #l + 1]
  //d_i is [which <- index_d, Lout_i <- #Lout_i]
  Tree::TupleExpr::TuplePairs odometerDims;

  Tree::TupleExpr::TuplePairs outPairs;
  //generate the Lout_i pairs
  for (dimension_index d : m_Lout)
  {
    outPairs.push_back(std::make_pair(Tree::DimensionExpr(d),
      Tree::HashExpr(Tree::DimensionExpr(d), false)));
  }

  //generate a unique "which" for each dimension
  for (const auto& v : e.dims)
  {
    int next = m_system.nextHiddenDim();
    w.whichDims.push_back(next);
    odometerDims.push_back
    (
      std::make_pair(Tree::IdentExpr(v.first), apply_visitor(*this, v.second))
    );

    //generate the dimExpr
    Tree::TupleExpr::TuplePairs dimTuple
      {{Tree::DimensionExpr(U"which"), mpz_class(next)}}
    ;

    dimTuple.insert(dimTuple.end(), outPairs.begin(), outPairs.end());

    //make a new var d = dimval
    m_newVars.push_back
    (
      Parser::Equation(v.first, Tree::Expr(), Tree::Expr(), 
        Tree::TupleExpr(dimTuple)
      )
    );
  }

  //generate the E which sets L_in dimension to 0
  for (auto dim : myLin)
  {
    odometerDims.push_back(
      std::make_pair(Tree::DimensionExpr(dim), mpz_class(0)));
  }

  //increment our own dim
  #if 0
  Tree::Expr incOwnRaw = Tree::BinaryOpExpr
    (
      Tree::BinaryOperator(Tree::ASSOC_LEFT, U"plus", U"+", 0),
      Tree::HashExpr(Tree::DimensionExpr(label), false),
      mpz_class(1)
    );
  #endif
  Tree::Expr incOwnRaw = Tree::BangAppExpr(
    Tree::BaseAbstractionExpr(U"intmp_plus"),
    {
      Tree::HashExpr(Tree::DimensionExpr(label), false),
      mpz_class(1)
    }
  );

  //need to do the whole tree fixup here
  auto fixed = fixupTree(m_system, incOwnRaw);
  Tree::Expr& incOwn = fixed.first;
  m_newVars.insert(m_newVars.end(), 
    fixed.second.equations.begin(), fixed.second.equations.end());

  w.e = Tree::AtExpr
    (
      Tree::AtExpr(expr, Tree::TupleExpr(odometerDims)),
      Tree::TupleExpr({{Tree::DimensionExpr(label), incOwn}})
    );

  //we return the rewritten E and add the variables to the list of variables
  //to add to the system

  //also add ourselves to the L_in to return
  myLin.push_back(label);
  m_Lin = std::move(myLin);

  //restore L_out
  m_Lout.pop_back();

  return w;
}

Tree::Expr
SemanticTransform::operator()(const Tree::IdentExpr& e)
{
  auto iter = m_lambdaScope.find(e.text);
  if (iter != m_lambdaScope.end())
  {
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
  inten.scope = m_Lin;
  inten.scope.insert(inten.scope.end(), m_Lout.begin(), m_Lout.end());

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
    m_system.nextHiddenDim() : argDim;

  m_lambdaScope.insert({e.name, argDim});

  //2. store our scope dimensions and ourself
  expr.scope = m_scope;
  expr.argDim = argDim;

  //3. add ourselves to the scope
  m_scope.push_back(argDim);
  m_scopeNames.push_back(e.name);

  //add our free variables to the scope
  for (const auto& free : e.free)
  {
    m_scope.push_back(free.second);
  }

  //4. visit the child
  expr.rhs = apply_visitor(*this, expr.rhs);

  //5. restore the scope
  //m_scope.pop_back();
  m_scope.resize(m_scope.size() - e.free.size() - 1);
  m_lambdaScope.erase(e.name);

  return expr;
}

Tree::Expr
SemanticTransform::operator()(const Tree::PhiExpr& e)
{
  throw "error: SemanticTransform(PhiExpr) reached";
}

}
