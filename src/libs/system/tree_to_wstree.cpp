/* Simplifies the Expr trees.
   Copyright (C) 2011 Jarryd Beck

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

/** @file tree_to_wstree.cpp
 * Rewrites an expression tree to remove operators and literals.
 * Also annotates tree with labels.
 */

#include <tl/fixed_indexes.hpp>
#include <tl/internal_strings.hpp>
#include <tl/parser_api.hpp>
#include <tl/rename.hpp>
#include <tl/system.hpp>
#include <tl/tree_to_wstree.hpp>

#include "tl/free_variables.hpp"

#include <sstream>

namespace TransLucid
{

void
TreeToWSTree::clear()
{
  //clear everything because the object might be reused
  m_Lout.clear();
  m_Lin.clear();
  m_namedAllScopeArgs.clear();
  m_namedAllScopeOdometers.clear();
}

template <typename... Args>
Tree::Expr
TreeToWSTree::rename(const Tree::Expr& expr, Args&&... args)
{
  RenameIdentifiers rename(*m_system, std::forward<Args>(args)...);
  Tree::Expr exprRenamed = rename.rename(expr);

  m_lastRenamed = rename.takeLastRenamed();

  return exprRenamed;
}

Tree::Expr
TreeToWSTree::toWSTree(const Tree::Expr& expr)
{
  clear();

  Tree::Expr exprRenamed = rename(expr);

  return apply_visitor(*this, exprRenamed);
}

Tree::Expr
TreeToWSTree::toWSTree(const Tree::Expr& expr, 
  const RenameIdentifiers::RenameRules& initial)
{
  clear();

  Tree::Expr exprRenamed = rename(expr, initial);

  return apply_visitor(*this, exprRenamed);
}

Tree::Expr
TreeToWSTree::operator()(const Tree::nil& n)
{
  return n;
}

Tree::Expr
TreeToWSTree::operator()(bool b)
{
  return b;
}

Tree::Expr
TreeToWSTree::operator()(Special s)
{
  return s;
}

Tree::Expr
TreeToWSTree::operator()(const mpz_class& i)
{
  return i;
}

Tree::Expr
TreeToWSTree::operator()(char32_t c)
{
  return c;
}

Tree::Expr
TreeToWSTree::operator()(const u32string& s)
{
  return s;
}

Tree::Expr
TreeToWSTree::operator()(const Tree::LiteralExpr& e)
{
  //LITERAL @ [type : e.type, text : e.text]
  #if 0
  std::cerr << "Translating LiteralExpr " << e.type << "\"" << e.text
            << "\" to" << std::endl
            << "LITERAL @ [typename <- \"" << e.type << "\", text <- \""
            << e.text << "\"]" << std::endl;
  #endif
  return Tree::AtExpr
  (
    Tree::IdentExpr(LITERAL_IDENT),
    Tree::TupleExpr
    (
      {
        {Tree::DimensionExpr(type_name_dim), e.type},
        {Tree::DimensionExpr(text_dim), e.text}
      }
    )
  );
}

Tree::Expr
TreeToWSTree::operator()(const Tree::DimensionExpr& e)
{
  return e;
}

Tree::Expr
TreeToWSTree::operator()(const Tree::IdentExpr& e)
{
  return e;
}

Tree::Expr
TreeToWSTree::operator()(const Tree::HashSymbol& e)
{
  return e;
}

Tree::Expr
TreeToWSTree::operator()(const Tree::ParenExpr& e)
{
  return apply_visitor(*this, e.e);
}

Tree::Expr
TreeToWSTree::operator()(const Tree::UnaryOpExpr& e)
{
  //(FN1 ! (#arg0)) @ [fnname <- e.op.op, arg0 <- T(e.e)]

  Tree::DimensionExpr arg0(U"arg0");

  return 
  Tree::AtExpr
  (
    Tree::BangAppExpr
    (
      Tree::IdentExpr(FN1_IDENT),
      Tree::HashExpr(arg0)
    ),
    Tree::TupleExpr
    (
      {
        {Tree::DimensionExpr(fnname_dim), e.op.op},
        {arg0, apply_visitor(*this, e.e)}
      }
    )
  );
}

Tree::Expr
TreeToWSTree::operator()(const Tree::BinaryOpExpr& e)
{
  //(e.op.op) . (e.lhs) . (e.rhs)
  //(FN2 ! (#arg0, #arg1))
  //  @ [fnname <- e.op.op, arg0 <- T(e.lhs), arg1 <- T(e.rhs)]

  Tree::Expr elhs = apply_visitor(*this, e.lhs);
  Tree::Expr erhs = apply_visitor(*this, e.rhs);

  Tree::Expr result = Tree::LambdaAppExpr
  (
    Tree::LambdaAppExpr(Tree::IdentExpr(e.op.op), e.lhs),
    e.rhs
  );

  return apply_visitor(*this, result);

  #if 0
  Tree::DimensionExpr arg0(U"arg0");
  Tree::DimensionExpr arg1(U"arg1");

  return 
  
  Tree::AtExpr
  (
    Tree::BangAppExpr
    (
      Tree::IdentExpr(FN2_IDENT),
      {
        Tree::HashExpr(arg0),
        Tree::HashExpr(arg1)
      }
    ),
    Tree::TupleExpr
    (
      {
        {Tree::DimensionExpr(U"arg0"), elhs},
        {Tree::DimensionExpr(U"arg1"), erhs},
        {Tree::DimensionExpr(fnname_dim), e.op.op}
      }
    )
  );
  #endif
}

Tree::Expr
TreeToWSTree::operator()(const Tree::BangAppExpr& e)
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
TreeToWSTree::operator()(const Tree::IfExpr& e)
{
  //do the elseifs
  std::vector<std::pair<Tree::Expr, Tree::Expr>> else_ifs;
  for (auto p : e.else_ifs)
  {
    else_ifs.push_back(std::make_pair(
      apply_visitor(*this, p.first),
      apply_visitor(*this, p.second)
    ));
  }

  return Tree::IfExpr(
    apply_visitor(*this, e.condition),
    apply_visitor(*this, e.then),
    else_ifs,
    apply_visitor(*this, e.else_)
  );
}

Tree::Expr
TreeToWSTree::operator()(const Tree::HashExpr& e)
{
  return Tree::HashExpr(apply_visitor(*this, e.e));
}

Tree::Expr
TreeToWSTree::operator()(const Tree::TupleExpr& e)
{
  std::vector<std::pair<Tree::Expr, Tree::Expr>> tuple;
  for (auto p : e.pairs)
  {
    tuple.push_back(std::make_pair(
      apply_visitor(*this, p.first),
      apply_visitor(*this, p.second)
    ));
  }

  return Tree::TupleExpr(tuple);
}

Tree::Expr
TreeToWSTree::operator()(const Tree::AtExpr& e)
{
  return Tree::AtExpr(
    apply_visitor(*this, e.lhs),
    apply_visitor(*this, e.rhs)
  );
}

Tree::Expr 
TreeToWSTree::operator()(const Tree::LambdaExpr& e)
{
  //1. generate a new dimension
  //2. store our scope dimensions
  //3. add ourselves to the scope
  //4. visit the child
  //5. add a new equation param = #dim
  //6. restore the scope

  FreeVariableReplacer replacer(*m_system);
  replacer.addBound(m_scopeNames);
  Tree::LambdaExpr expr = get<Tree::LambdaExpr>(replacer.replaceFree(e));

  auto& replaced = replacer.getReplaced();

  //1. generate a new dimension
  dimension_index argDim = m_system->nextHiddenDim();

  //2. store our scope dimensions and ourself
  expr.scope = m_scope;
  expr.argDim = argDim;

  //3. add ourselves to the scope
  m_scope.push_back(argDim);
  m_scopeNames.push_back(e.name);

  //add the dimensions to replace with to the scope
  for (const auto& r : replaced)
  {
    m_scope.push_back(r.second);
  }

  //4. visit the child
  expr.rhs = apply_visitor(*this, expr.rhs);

  //5. add a new equation param = #dim
  m_newVars.push_back
  (
    std::make_tuple
    (
      e.name,
      Tree::Expr(),
      Tree::Expr(),
      Tree::HashExpr(Tree::DimensionExpr(argDim))
    )
  );

  //6. restore the scope
  //m_scope.pop_back();
  m_scope.resize(m_scope.size() - replaced.size() - 1);

  //store the free variables to evaluate
  expr.free = std::move(replaced);

  return expr;
}

Tree::Expr
TreeToWSTree::operator()(const Tree::PhiExpr& e)
{
  //1. generate a new dimension
  //2. store our scope dimensions and ourself
  //3. add ourselves to the scope
  //4. visit the child
  //5. add a new equation name = args @ [stuff]
  //6. restore the scope

  FreeVariableReplacer replacer(*m_system);
  replacer.addBound(m_scopeNames);
  Tree::PhiExpr expr = get<Tree::PhiExpr>(replacer.replaceFree(e));

  auto& replaced = replacer.getReplaced();

  //1. generate new dimensions
  dimension_index argDim = m_system->nextHiddenDim();
  dimension_index odometerDim = m_system->nextHiddenDim();

  //2. store our scope dimensions and ourself
  expr.scope = m_scope;

  expr.argDim = argDim;
  expr.odometerDim = odometerDim;

  //3. add ourselves to the scope
  m_scopeNames.push_back(e.name);
  m_scope.push_back(argDim);
  m_scope.push_back(odometerDim);

  //add the dimensions to replace with to the scope
  for (const auto& r : replaced)
  {
    m_scope.push_back(r.second);
  }

  //also the all scope
  m_namedAllScopeArgs.push_back(argDim);
  m_namedAllScopeOdometers.push_back(odometerDim);

  //4. visit the child
  expr.rhs = apply_visitor(*this, expr.rhs);

  //5. add a new equation name = args @ [stuff]
  m_newVars.push_back(Parser::Equation
  (
    e.name,
    Tree::Expr(),
    Tree::Expr(),
    Tree::AtExpr(
      Tree::IdentExpr(U"args"),
      Tree::TupleExpr(Tree::TupleExpr::TuplePairs
      {
        {
          Tree::DimensionExpr(DIM_PI), 
          Tree::DimensionExpr(odometerDim)
        },
        {
          Tree::DimensionExpr(DIM_PSI),
          Tree::DimensionExpr(argDim)
        }
      }
      )
    )
  ));

  //6. restore the scope
  //we have two things to restore, the odometer and the two args
  m_scope.resize(m_scope.size() - replaced.size() - 2);

  expr.free = std::move(replaced);

  return expr;
}

Tree::Expr
TreeToWSTree::operator()(const Tree::LambdaAppExpr& e)
{
  return Tree::LambdaAppExpr(
    apply_visitor(*this, e.lhs),
    apply_visitor(*this, e.rhs)
  );
}

Tree::Expr
TreeToWSTree::operator()(const Tree::PhiAppExpr& e)
{
  //store the dims of L_all

  Tree::PhiAppExpr expr
  (
    apply_visitor(*this, e.lhs),
    apply_visitor(*this, e.rhs)
  );

  expr.Lall = m_Lout;
  expr.Lall.insert(expr.Lall.end(), m_Lin.begin(), m_Lin.end());

  return expr;
}

Tree::Expr
TreeToWSTree::operator()(const Tree::WhereExpr& e)
{
  //all the names are already unique
  Tree::WhereExpr w = e;

  std::vector<dimension_index> myLin;
  
  //generate new label
  dimension_index label = m_system->nextHiddenDim();
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
      Tree::HashExpr(Tree::DimensionExpr(d))));
  }

  //generate a unique "which" for each dimension
  for (const auto& v : e.dims)
  {
    int next = m_system->nextHiddenDim();
    w.whichDims.push_back(next);
    odometerDims.push_back
    (
      std::make_pair(Tree::IdentExpr(v.first), v.second)
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
  Tree::Expr incOwnRaw = Tree::BinaryOpExpr
    (
      Tree::BinaryOperator(Tree::ASSOC_LEFT, U"plus", U"+", 0),
      Tree::HashExpr(Tree::DimensionExpr(label)),
      mpz_class(1)
    );

  Tree::Expr incOwn = apply_visitor(*this, incOwnRaw);

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

//the free variable replacer code
//whenever this is called everything has been renamed, so there is
//no problem with name clashes

Tree::Expr 
FreeVariableReplacer::operator()(const Tree::IdentExpr& e)
{
  if (m_bound.find(e.text) == m_bound.end())
  {
    dimension_index unique = m_system.nextHiddenDim();
    m_replaced.push_back(std::make_pair(e.text, unique));
    return Tree::HashExpr(Tree::DimensionExpr(unique));
  }
  else
  {
    return e;
  }
}

Tree::Expr 
FreeVariableReplacer::operator()(const Tree::LambdaExpr& e)
{
  m_bound.insert(e.name);
  Tree::Expr replaced = apply_visitor(*this, e.rhs);
  m_bound.erase(e.name);

  return Tree::LambdaExpr(e.name, std::move(replaced));
}

Tree::Expr 
FreeVariableReplacer::operator()(const Tree::PhiExpr& e)
{
  m_bound.insert(e.name);
  Tree::Expr replaced = apply_visitor(*this, e.rhs);
  m_bound.erase(e.name);

  return Tree::PhiExpr(e.name, std::move(replaced));
}

Tree::Expr 
FreeVariableReplacer::operator()(const Tree::WhereExpr& e)
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

  where.e = e.e;

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
FreeVariableReplacer::replaceFree(const Tree::Expr& expr)
{
  m_replaced.clear();
  return apply_visitor(*this, expr);
}

}
