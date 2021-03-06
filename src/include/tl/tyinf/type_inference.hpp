/* Type inference algorithm.
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

#ifndef TL_TYINF_TYPE_INFERENCE_HPP_INCLUDED
#define TL_TYINF_TYPE_INFERENCE_HPP_INCLUDED

#include <tl/ast_fwd.hpp>
#include <tl/tyinf/constraint_graph.hpp>
#include <tl/tyinf/type.hpp>
#include <tl/tyinf/type_context.hpp>
#include <tl/tyinf/type_variable.hpp>

namespace TransLucid
{
  class TypeRegistry;

  namespace TypeInference
  {
    typedef std::tuple<TypeContext, Type, ConstraintGraph> TypeScheme;

    std::tuple<u32string, u32string, u32string>
    display_type_scheme(const TypeScheme& t, System& system);

    class TypeInferrer
    {
      public:
      typedef TypeScheme result_type;

      TypeInferrer(System& system, FreshTypeVars& freshVars)
      : m_freshVars(freshVars)
      , m_system(system)
      , indecl(false)
      {
      }

      TypeVariable
      fresh()
      {
        return m_freshVars.fresh();
      }

      void
      infer_system(const std::set<u32string>& ids);

      result_type
      infer(const Tree::Expr& e);

      //the actual type, the context (the type will be TypeNothing here)
      std::pair<TypeScheme, TypeScheme>
      separate_context(const TypeScheme& t);
      
      result_type
      operator()(const Tree::nil&);

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
      operator()(const Constant& c);

      result_type
      operator()(const Tree::LiteralExpr&);

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

      const std::map<u32string, TypeScheme>&
      environment() const
      {
        return m_environment;
      }

      void
      setType(const u32string& x, const TypeScheme& S)
      {
        m_environment[x] = S;
      }

      private:

      std::vector<std::vector<u32string>>
      generate_recurse_groups(const std::set<u32string>& ids);

      TypeScheme
      simplify(TypeScheme t);

      std::pair<TypeContext, ConstraintGraph>
      process_region_guard
      (
        const Tree::RegionExpr& r,
        std::vector<std::pair<dimension_index, Type>>& result
      );

      FreshTypeVars& m_freshVars;

      template <typename T>
      result_type
      make_constant(T&& v);

      System& m_system;

      bool indecl;

      std::map<u32string, TypeScheme> m_environment;
    };

    TypeAtomic
    makeAtomic(TypeRegistry& reg, const u32string& name);

    TypeScheme
    canonise(const TypeScheme& t, FreshTypeVars& fresh);

    std::pair<VarSet, VarSet>
    polarity(const TypeScheme& t);

    TypeScheme
    garbage_collect(const TypeScheme& t);

    TypeScheme
    minimise(const TypeScheme& t);

    TypeScheme
    display_type(const TypeScheme& t);
  }
}

#endif
