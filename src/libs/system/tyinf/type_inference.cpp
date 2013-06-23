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

#include <tl/fixed_indexes.hpp>
#include <tl/tyinf/type_inference.hpp>

#include <tl/types/boolean.hpp>
#include <tl/types/char.hpp>
#include <tl/types/intmp.hpp>
#include <tl/types/string.hpp>
#include <tl/types/special.hpp>

namespace TransLucid
{
namespace TypeInference
{

template <typename T>
TypeInferrer::result_type
TypeInferrer::make_constant(T&& v)
{
  TypeVariable t = fresh();

  ConstraintGraph c;

  c.add_to_closure(Constraint{t, std::forward<T>(v)});

  return std::make_tuple(TypeContext(), t, c);
}

TypeInferrer::result_type
TypeInferrer::infer(const Tree::Expr& e)
{
  apply_visitor(*this, e);
}

TypeInferrer::result_type
TypeInferrer::operator()(const Tree::nil& e)
{
  return result_type();
}

TypeInferrer::result_type
TypeInferrer::operator()(const bool& e)
{
  return make_constant(Types::Boolean::create(e));
}

TypeInferrer::result_type
TypeInferrer::operator()(const Special& e)
{
  return make_constant(Types::Special::create(e));
}

TypeInferrer::result_type
TypeInferrer::operator()(const mpz_class& e)
{
  return make_constant(Types::Intmp::create(e));
}

TypeInferrer::result_type
TypeInferrer::operator()(const char32_t& e)
{

}

TypeInferrer::result_type
TypeInferrer::operator()(const u32string& e)
{

}

TypeInferrer::result_type
TypeInferrer::operator()(const Tree::LiteralExpr& e)
{

}

TypeInferrer::result_type
TypeInferrer::operator()(const Tree::DimensionExpr& e)
{

}

TypeInferrer::result_type
TypeInferrer::operator()(const Tree::IdentExpr& e)
{

}

TypeInferrer::result_type
TypeInferrer::operator()(const Tree::HashSymbol& e)
{

}

TypeInferrer::result_type
TypeInferrer::operator()(const Tree::HostOpExpr& e)
{

}

TypeInferrer::result_type
TypeInferrer::operator()(const Tree::ParenExpr& e)
{

}

TypeInferrer::result_type
TypeInferrer::operator()(const Tree::UnaryOpExpr& e)
{

}

TypeInferrer::result_type
TypeInferrer::operator()(const Tree::BinaryOpExpr& e)
{

}

TypeInferrer::result_type
TypeInferrer::operator()(const Tree::MakeIntenExpr& e)
{

}

TypeInferrer::result_type
TypeInferrer::operator()(const Tree::EvalIntenExpr& e)
{

}

TypeInferrer::result_type
TypeInferrer::operator()(const Tree::IfExpr& e)
{

}

TypeInferrer::result_type
TypeInferrer::operator()(const Tree::HashExpr& e)
{

}

TypeInferrer::result_type
TypeInferrer::operator()(const Tree::RegionExpr& e)
{

}

TypeInferrer::result_type
TypeInferrer::operator()(const Tree::TupleExpr& e)
{

}

TypeInferrer::result_type
TypeInferrer::operator()(const Tree::AtExpr& e)
{

}

TypeInferrer::result_type
TypeInferrer::operator()(const Tree::LambdaExpr& e)
{
  auto t_0 = apply_visitor(*this, e.rhs);

  auto& C = std::get<2>(t_0);
  auto& context = std::get<0>(t_0);

  auto alpha = fresh();

  C.add_to_closure(Constraint{
    TypeCBV{context.lookup(e.argDim), std::get<1>(t_0)}, 
    alpha});

  context.remove(e.argDim);

  return std::make_tuple(context, alpha, C);
}

TypeInferrer::result_type
TypeInferrer::operator()(const Tree::PhiExpr& e)
{

}

TypeInferrer::result_type
TypeInferrer::operator()(const Tree::BaseAbstractionExpr& e)
{

}

TypeInferrer::result_type
TypeInferrer::operator()(const Tree::BangAppExpr& e)
{

}

TypeInferrer::result_type
TypeInferrer::operator()(const Tree::LambdaAppExpr& e)
{
  auto t_0 = apply_visitor(*this, e.lhs);
  auto t_1 = apply_visitor(*this, e.rhs);

  auto& context = std::get<0>(t_0);
  context.join(std::get<0>(t_1));

  auto& C = std::get<2>(t_0);
  C.make_union(std::get<2>(t_1));

  auto gamma = fresh();
  auto alpha = fresh();

  C.add_to_closure(Constraint{alpha, gamma});
  C.add_to_closure(Constraint{std::get<1>(t_0), 
    TypeCBV{std::get<1>(t_1), alpha}});

  return std::make_tuple(context, gamma, C);
}

TypeInferrer::result_type
TypeInferrer::operator()(const Tree::PhiAppExpr& e)
{

}

TypeInferrer::result_type
TypeInferrer::operator()(const Tree::WhereExpr& e)
{

}

TypeInferrer::result_type
TypeInferrer::operator()(const Tree::ConditionalBestfitExpr& e)
{

}

}
}
