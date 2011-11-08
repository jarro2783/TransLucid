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

#include <unordered_map>

#include <tl/ast.hpp>
#include <tl/system.hpp>

namespace TransLucid
{
  class RenameIdentifiers
  {
    public:

    //for boost::apply_visitor
    typedef Tree::Expr result_type;

    //a hashmap would be better, but we need to hold on to iterators
    //while we're changing it and they can't be invalidated
    typedef std::map<u32string, u32string> RenameRules;

    RenameIdentifiers(System& system);
    RenameIdentifiers(System& system, const RenameRules& startRules);

    template <typename T>
    Tree::Expr 
    operator()(const T& e)
    {
      return e;
    }

    Tree::Expr
    operator()(const Tree::IdentExpr& e);

    Tree::Expr
    operator()(const Tree::ParenExpr& e);

    Tree::Expr
    operator()(const Tree::UnaryOpExpr& e);

    Tree::Expr
    operator()(const Tree::BinaryOpExpr& e);

    Tree::Expr
    operator()(const Tree::IfExpr& e);

    Tree::Expr
    operator()(const Tree::HashExpr& e);

    Tree::Expr
    operator()(const Tree::TupleExpr& e);

    Tree::Expr
    operator()(const Tree::AtExpr& e);

    Tree::Expr
    operator()(const Tree::WhereExpr& e);

    Tree::Expr 
    operator()(const Tree::BangExpr& e);

    Tree::Expr
    operator()(const Tree::LambdaExpr& e);

    Tree::Expr
    operator()(const Tree::PhiExpr& e);

    Tree::Expr
    operator()(const Tree::BangAppExpr& e);

    Tree::Expr
    operator()(const Tree::LambdaAppExpr& e);

    Tree::Expr
    operator()(const Tree::PhiAppExpr& e);

    Tree::Expr
    rename(const Tree::Expr& e)
    {
      return apply_visitor(*this, e);
    }

    std::vector<std::pair<Tree::Expr, Tree::Expr>>
    rename_list(const std::vector<std::pair<Tree::Expr, Tree::Expr>>& list);

    u32string
    renameString(const u32string& s);

    u32string
    generateUnique(const u32string& suffix);

    private:

    template <typename T>
    Tree::Expr
    renameFunction(const T& f);

    template <typename T>
    Tree::Expr
    renameFunApp(const T& app);

    RenameRules m_rules;

    System& m_system;
  };
}
