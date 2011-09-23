/* Rename identifiers so that they are unique.
   Copyright (C) 2011 Jarryd Beck and John Plaice

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

#include <sstream>

#include <tl/rename.hpp>

namespace TransLucid
{

RenameIdentifiers::RenameIdentifiers(System& system)
: m_system(system)
{
}

RenameIdentifiers::RenameIdentifiers
(
  System& system, 
  const RenameRules& startRules
)
: m_rules(startRules)
, m_system(system)
{
}

Tree::Expr
RenameIdentifiers::operator()(const Tree::IdentExpr& e)
{
  auto iter = m_rules.find(e.text);

  if (iter != m_rules.end())
  {
    return Tree::IdentExpr(iter->second);
  }
  else
  {
    return e;
  }
}

Tree::Expr
RenameIdentifiers::operator()(const Tree::ParenExpr& e)
{
  return Tree::ParenExpr(boost::apply_visitor(*this, e.e));
}

Tree::Expr
RenameIdentifiers::operator()(const Tree::UnaryOpExpr& e)
{
  return Tree::UnaryOpExpr(e.op, boost::apply_visitor(*this, e.e));
}

Tree::Expr
RenameIdentifiers::operator()(const Tree::BinaryOpExpr& e)
{
  return Tree::BinaryOpExpr
  (
    e.op,
    boost::apply_visitor(*this, e.lhs),
    boost::apply_visitor(*this, e.rhs)
  );
}

Tree::Expr
RenameIdentifiers::operator()(const Tree::IfExpr& e)
{
  return Tree::IfExpr
  (
    boost::apply_visitor(*this, e.condition),
    boost::apply_visitor(*this, e.then),
    rename_list(e.else_ifs),
    boost::apply_visitor(*this, e.else_)
  );
}

Tree::Expr
RenameIdentifiers::operator()(const Tree::HashExpr& e)
{
  return Tree::HashExpr(boost::apply_visitor(*this, e.e));
}

Tree::Expr
RenameIdentifiers::operator()(const Tree::TupleExpr& e)
{
  std::vector<std::pair<Tree::Expr, Tree::Expr>> renamed;

  for (auto iter = e.pairs.begin(); iter != e.pairs.end(); ++iter)
  {
    renamed.push_back(std::make_pair
    (
      boost::apply_visitor(*this, iter->first),
      boost::apply_visitor(*this, iter->second)
    ));
  }

  return renamed;
}

Tree::Expr
RenameIdentifiers::operator()(const Tree::AtExpr& e)
{
  return Tree::AtExpr
  (
    boost::apply_visitor(*this, e.lhs),
    boost::apply_visitor(*this, e.rhs)
  );
}

Tree::Expr
RenameIdentifiers::operator()(const Tree::WhereExpr& e)
{
  //1. Rename all dimension expressions because they are outside the scope of
  //the where clause.
  //2. Generate new names for dimensions and variables. If a variable shadows
  //something already to be renamed, remember it and then change it for step 3.
  //3. Rename E and inside variable definitions.
  //4. Restore the shadowed names.
  Tree::WhereExpr w;
  RenameRules shadowed;

  RenameRules newNames;

  //first rename in dims because their scope is outside the where
  for (const auto& dim : e.dims)
  {
    //generate a unique name and store in the new tree at the same time

    //if there are multiple definitions of the same dimension then don't
    //break here, just let it break at bestfitting later
    //we can detect this in the parser if we want
    auto iter = newNames.find(dim.first);

    u32string uniqueDim;
    if (iter != newNames.end())
    {
      uniqueDim = iter->second;
    }
    else
    {
      uniqueDim = generateUnique(U"uniquedim");
      newNames.insert(std::make_pair(dim.first, uniqueDim));
    }

    w.dims.push_back
    (
      std::make_pair(uniqueDim, boost::apply_visitor(*this, dim.second))
    );
  }

  //go through all of the dimensions, they are renamed inside variables
  //and the E. These could shadow existing names.
  for (const auto& dimRename : newNames)
  {
    auto iter = m_rules.find(dimRename.first); 
    if (iter != m_rules.end())
    {
      shadowed.insert(dimRename);
      iter->second = dimRename.second;
    }
    else
    {
      m_rules.insert(dimRename);
    }
  }

  //for every variable, generate a new name
  //Multiple definitions of the same variable are perfectly acceptable.
  //Even variables with the same name as dimensions, this is probably wrong too
  //but someone may or may not pick it up somewhere and bestfitting should take
  //care of it
  for (const auto& var : e.vars)
  {
    u32string unique;
    const u32string& original = std::get<0>(var);

    auto newIter = newNames.find(original);
    if (newIter != newNames.end())
    {
      unique = newIter->second;
    }
    else
    {
      unique = generateUnique(U"uniquevar");
      newIter = newNames.insert(std::make_pair(original, unique)).first;
      
      //if it shadows an existing name, but we only care if it is the first
      //time that we have seen this variable
      auto rulesIter = m_rules.find(original);
      if (rulesIter != m_rules.end())
      {
        shadowed.insert(*rulesIter);
        rulesIter->second = unique;
      }
      else
      {
        m_rules.insert(*newIter);
      }
    }
  }

  //then do some renaming

  //rename expr
  w.e = boost::apply_visitor(*this, e.e);
  
  //rename the vars
  for (const auto& var : e.vars)
  {
    //rename inside every subexpr of a variable
    Tree::Expr guard = boost::apply_visitor(*this, std::get<1>(var));
    Tree::Expr boolean = boost::apply_visitor(*this, std::get<2>(var));
    Tree::Expr expr = boost::apply_visitor(*this, std::get<3>(var));

    w.vars.push_back
    (
      std::make_tuple(renameString(std::get<0>(var)), guard, boolean, expr)
    );
  }

  //delete the dim names and var names, then reinsert the shadowed
  m_rules.erase(newNames.begin(), newNames.end());
  m_rules.insert(shadowed.begin(), shadowed.end());

  return w;
}

template <typename T>
Tree::Expr
RenameIdentifiers::renameFunction(const T& f)
{
  //generate a new name
  u32string unique = generateUnique(U"uniquefn");

  //if the name shadows an existing name then store it
  u32string shadowed;

  auto iter = m_rules.find(f.name);
  if (iter != m_rules.end())
  {
    shadowed = iter->second;
    iter->second = unique;
  }
  else
  {
    iter = m_rules.insert(std::make_pair(f.name, unique)).first;
  }

  //rename
  T l(f);
  l.name = unique;
  l.rhs = boost::apply_visitor(*this, f.rhs);

  //restore the shadowed name
  if (!shadowed.empty())
  {
    iter->second = shadowed;
  }
  else
  {
    m_rules.erase(iter);
  }

  return l;
}

Tree::Expr
RenameIdentifiers::operator()(const Tree::PhiExpr& e)
{
  return renameFunction(e);
}

Tree::Expr
RenameIdentifiers::operator()(const Tree::LambdaExpr& e)
{
  return renameFunction(e);
}

template <typename T>
Tree::Expr
RenameIdentifiers::renameFunApp(const T& app)
{
  return T
  (
    boost::apply_visitor(*this, app.lhs),
    boost::apply_visitor(*this, app.rhs)
  );
}

Tree::Expr
RenameIdentifiers::operator()(const Tree::LambdaAppExpr& e)
{
  return renameFunApp(e);
}

Tree::Expr
RenameIdentifiers::operator()(const Tree::PhiAppExpr& e)
{
  return renameFunApp(e);
}

std::vector<std::pair<Tree::Expr, Tree::Expr>>
RenameIdentifiers::rename_list
(
  const std::vector<std::pair<Tree::Expr, Tree::Expr>>& list
)
{
  std::vector<std::pair<Tree::Expr, Tree::Expr>> renamed;

  for (auto iter = list.begin(); iter != list.end(); ++iter)
  {
    renamed.push_back(std::make_pair
      (
        boost::apply_visitor(*this, iter->first),
        boost::apply_visitor(*this, iter->second)
      )
    );
  }

  return renamed;
}

u32string
RenameIdentifiers::renameString(const u32string& s)
{
  auto iter = m_rules.find(s);
  if (iter != m_rules.end())
  {
    return iter->second;
  }

  return s;
}

u32string
RenameIdentifiers::generateUnique(const u32string& suffix)
{
  std::ostringstream os;
  os << m_system.nextVarIndex() << "_" << utf32_to_utf8(suffix);

  return to_u32string(os.str());
}

}