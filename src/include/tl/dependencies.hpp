/* Dependency checking.
   Copyright (C) 2013 Jarryd Beck

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

#include <tl/ast.hpp>
#include <tl/static/function.hpp>
#include <tl/system.hpp>

namespace TransLucid
{
  namespace Static
  {
    class DependencyFinder
    {
      public:
      typedef std::set<u32string> IdentifierSet;
      typedef Static::Functions::FunctorList<IdentifierSet> FunctorList;
      typedef Static::Functions::FunctorSet<IdentifierSet> FunctorSet;

      //0. the identifiers depended on, 1. the direct function values,
      //2. the indirect function values
      typedef std::tuple<IdentifierSet, FunctorList, FunctorSet> result_type;

      // function types
      typedef Static::Functions::Up<IdentifierSet> Up;
      typedef Static::Functions::CBV<IdentifierSet> CBV;
      typedef Static::Functions::Base<IdentifierSet> Base;
      typedef Static::Functions::Down<IdentifierSet> Down;
      typedef Static::Functions::ApplyV<IdentifierSet> ApplyV;
      typedef Static::Functions::ApplyB<IdentifierSet> ApplyB;

      typedef std::map<u32string, result_type> DependencyMap;

      DependencyFinder(System* s)
      : m_system(s)
      {
      }

      DependencyMap
      computeDependencies();

      result_type
      operator()(const Tree::nil&)
      {
        return result_type();
      }

      result_type
      operator()(const bool&);

      result_type
      operator()(const Special&);

      result_type
      operator()(const mpz_class&);

      result_type
      operator()(const char32_t&);

      result_type
      operator()(const u32string&);

      result_type
      operator()(const Tree::LiteralExpr&);

      result_type
      operator()(const Constant&);

      result_type
      operator()(const Tree::DimensionExpr&);

      result_type
      operator()(const Tree::IdentExpr&);

      result_type
      operator()(const Tree::HashSymbol&);

      result_type
      operator()(const Tree::HostOpExpr&);

      result_type
      operator()(const Tree::ParenExpr&);

      result_type
      operator()(const Tree::UnaryOpExpr&);

      result_type
      operator()(const Tree::BinaryOpExpr&);

      result_type
      operator()(const Tree::MakeIntenExpr&);

      result_type
      operator()(const Tree::EvalIntenExpr&);

      result_type
      operator()(const Tree::IfExpr&);

      result_type
      operator()(const Tree::HashExpr&);

      result_type
      operator()(const Tree::RegionExpr&);

      result_type
      operator()(const Tree::TupleExpr&);

      result_type
      operator()(const Tree::AtExpr&);

      result_type
      operator()(const Tree::LambdaExpr&);

      result_type
      operator()(const Tree::PhiExpr&);

      result_type
      operator()(const Tree::BaseAbstractionExpr&);

      result_type
      operator()(const Tree::BangAppExpr&);

      result_type
      operator()(const Tree::LambdaAppExpr&);

      result_type
      operator()(const Tree::PhiAppExpr&);

      result_type
      operator()(const Tree::WhereExpr&);

      result_type
      operator()(const Tree::ConditionalBestfitExpr&);

      private:
      
      DependencyMap m_idDeps;
      System* m_system;
    };

    namespace Functions
    {
      template <>
      struct PropertyCounter<DependencyFinder::IdentifierSet>
      {
        size_t
        operator()(const DependencyFinder::IdentifierSet& s)
        {
          return s.size();
        }
      };
    }
  }
}
