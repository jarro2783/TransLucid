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

#include <tl/parser_api.hpp>
#include <tl/rename.hpp>
#include <tl/system.hpp>
#include <tl/tree_to_wstree.hpp>
#include <tl/internal_strings.hpp>

#include <sstream>

namespace TransLucid
{

Tree::Expr
TreeToWSTree::toWSTree(const Tree::Expr& expr)
{
  //rename everything first
  RenameIdentifiers rename(*m_system);

  Tree::Expr exprRenamed = rename.rename(expr);

  return boost::apply_visitor(*this, exprRenamed);
}

Tree::Expr TreeToWSTree::operator()(const Tree::nil& n)
{
  return n;
}

Tree::Expr TreeToWSTree::operator()(bool b)
{
  return b;
}

Tree::Expr TreeToWSTree::operator()(Special s)
{
  return s;
}

Tree::Expr TreeToWSTree::operator()(const mpz_class& i)
{
  return i;
}

Tree::Expr TreeToWSTree::operator()(char32_t c)
{
  return c;
}

Tree::Expr TreeToWSTree::operator()(const u32string& s)
{
  return s;
}

Tree::Expr TreeToWSTree::operator()(const Tree::LiteralExpr& e)
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

Tree::Expr TreeToWSTree::operator()(const Tree::DimensionExpr& e)
{
  return e;
}

Tree::Expr TreeToWSTree::operator()(const Tree::IdentExpr& e)
{
  return e;
}

Tree::Expr TreeToWSTree::operator()(const Tree::ParenExpr& e)
{
  return boost::apply_visitor(*this, e.e);
}

Tree::Expr TreeToWSTree::operator()(const Tree::UnaryOpExpr& e)
{
  //(FN1 ! (#arg0)) @ [fnname <- e.op.op, arg0 <- T(e.e)]

  //for now just get this working, we can optimise by only calculating the
  //expr once, but how do I do that...
  //
  Tree::DimensionExpr arg0(U"arg0");

  return 
  Tree::AtExpr
  (
    Tree::BangOpExpr
    (
      Tree::IdentExpr(FN1_IDENT),
      {Tree::HashExpr(arg0)}
    ),
    Tree::TupleExpr
    (
      {
        {Tree::DimensionExpr(fnname_dim), e.op.op},
        {arg0, boost::apply_visitor(*this, e.e)}
      }
    )
  );

  #if 0
  //UNOP @ [arg0 <- e.e, opname <- e.op.op]
  return Tree::AtExpr
  (
    Tree::IdentExpr(U"UNOP"),
    Tree::TupleExpr
    (
      {
        {Tree::DimensionExpr(U"arg0"), boost::apply_visitor(*this, e.e)},
        {Tree::DimensionExpr(U"opname"), e.op.op}
      }
    )
  );
  #endif
}

Tree::Expr TreeToWSTree::operator()(const Tree::BinaryOpExpr& e)
{
  //(FN2 ! (#arg0, #arg1))
  //  @ [fnname <- e.op.op, arg0 <- T(e.lhs), arg1 <- T(e.rhs)]

  Tree::Expr elhs = boost::apply_visitor(*this, e.lhs);
  Tree::Expr erhs = boost::apply_visitor(*this, e.rhs);

  Tree::DimensionExpr arg0(U"arg0");
  Tree::DimensionExpr arg1(U"arg1");

  //optimise as above
  return 
  
  Tree::AtExpr
  (
    Tree::BangOpExpr
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


  #if 0
  //BINOP @ [arg0 <- e.lhs, arg1 <- e.rhs, opname <- e.op.op]
  return Tree::AtExpr
  (
    Tree::IdentExpr(U"BINOP"),
    Tree::TupleExpr
    (
      {
        {Tree::DimensionExpr(U"arg0"), boost::apply_visitor(*this, e.lhs)},
        {Tree::DimensionExpr(U"arg1"), boost::apply_visitor(*this, e.rhs)},
        {Tree::DimensionExpr(U"opname"), e.op.op}
      }
    )
  );
  #endif
}

Tree::Expr
TreeToWSTree::operator()(const Tree::BangOpExpr& e)
{
  Tree::Expr name = boost::apply_visitor(*this, e.name);
  std::vector<Tree::Expr> args;

  for (auto expr : e.args)
  {
    args.push_back(boost::apply_visitor(*this, expr));
  }

  return Tree::BangOpExpr(name, args);
}

Tree::Expr TreeToWSTree::operator()(const Tree::IfExpr& e)
{
  //do the elseifs
  std::vector<std::pair<Tree::Expr, Tree::Expr>> else_ifs;
  for (auto p : e.else_ifs)
  {
    else_ifs.push_back(std::make_pair(
      boost::apply_visitor(*this, p.first),
      boost::apply_visitor(*this, p.second)
    ));
  }

  return Tree::IfExpr(
    boost::apply_visitor(*this, e.condition),
    boost::apply_visitor(*this, e.then),
    else_ifs,
    boost::apply_visitor(*this, e.else_)
  );
}

Tree::Expr TreeToWSTree::operator()(const Tree::HashExpr& e)
{
  return Tree::HashExpr(boost::apply_visitor(*this, e.e));
}

Tree::Expr TreeToWSTree::operator()(const Tree::TupleExpr& e)
{
  std::vector<std::pair<Tree::Expr, Tree::Expr>> tuple;
  for (auto p : e.pairs)
  {
    tuple.push_back(std::make_pair(
      boost::apply_visitor(*this, p.first),
      boost::apply_visitor(*this, p.second)
    ));
  }

  return Tree::TupleExpr(tuple);
}

Tree::Expr TreeToWSTree::operator()(const Tree::AtExpr& e)
{
  return Tree::AtExpr(
    boost::apply_visitor(*this, e.lhs),
    boost::apply_visitor(*this, e.rhs)
  );
}

Tree::Expr TreeToWSTree::operator()(const Tree::PhiExpr& e)
{
  return e;
}

Tree::Expr TreeToWSTree::operator()(const Tree::LambdaExpr& e)
{
  return Tree::LambdaExpr(e.name, boost::apply_visitor(*this, e.rhs));
}

Tree::Expr TreeToWSTree::operator()(const Tree::PhiAppExpr& e)
{
  return Tree::PhiAppExpr(
    boost::apply_visitor(*this, e.lhs),
    boost::apply_visitor(*this, e.rhs)
  );
}

Tree::Expr TreeToWSTree::operator()(const Tree::LambdaAppExpr& e)
{
  return Tree::LambdaAppExpr(
    boost::apply_visitor(*this, e.lhs),
    boost::apply_visitor(*this, e.rhs)
  );
}

Tree::Expr TreeToWSTree::operator()(const Tree::WhereExpr& e)
{
  //all the names are already unique
  Tree::WhereExpr w = e;

  std::vector<dimension_index> myLin;
  
  //generate new label
  dimension_index label = m_system->nextDimensionIndex();
  w.myDim = label;
  //store L_out
  //visit children variables
  m_Lout.push_back(label);

  //L_in is all of the inner where clauses 
  //boost::apply_visitor(*this, e.e);

  m_Lin.clear();
  for (const auto& evar : e.vars)
  {
    //expr
    m_Lin.clear();
    Tree::Expr varExpr = boost::apply_visitor(*this, std::get<3>(evar));
    myLin.insert(myLin.end(), m_Lin.begin(), m_Lin.end());

    //guard
    m_Lin.clear();
    Tree::Expr guardExpr = boost::apply_visitor(*this, std::get<1>(evar));
    myLin.insert(myLin.end(), m_Lin.begin(), m_Lin.end());

    //boolean
    m_Lin.clear();
    Tree::Expr booleanExpr = boost::apply_visitor(*this, std::get<2>(evar));;
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
  Tree::Expr expr = boost::apply_visitor(*this, e.e);
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
    int next = m_system->nextDimIndex();
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

  Tree::Expr incOwn = boost::apply_visitor(*this, incOwnRaw);

  #if 0
  odometerDims.push_back
  (
    std::make_pair
    (
      Tree::DimensionExpr(label),
      incOwn
    )
  );
  #endif

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

}
