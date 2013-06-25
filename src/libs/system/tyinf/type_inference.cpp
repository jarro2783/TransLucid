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
#include <tl/types/dimension.hpp>
#include <tl/types/intmp.hpp>
#include <tl/types/string.hpp>
#include <tl/types/special.hpp>

#include <tl/system.hpp>

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
  return apply_visitor(*this, e);
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
  return make_constant(Types::UChar::create(e));
}

TypeInferrer::result_type
TypeInferrer::operator()(const u32string& e)
{
  return make_constant(Types::String::create(e));
}

TypeInferrer::result_type
TypeInferrer::operator()(const Tree::LiteralExpr& e)
{
  throw "Literal expression in type checker";
}

TypeInferrer::result_type
TypeInferrer::operator()(const Tree::DimensionExpr& e)
{
  if (e.text.empty())
  {
    return make_constant(Types::Dimension::create(e.dim));
  }
  else
  {
    return make_constant(
      Types::Dimension::create(m_system.getDimensionIndex(e.text)));
  }
}

TypeInferrer::result_type
TypeInferrer::operator()(const Tree::IdentExpr& e)
{

}

TypeInferrer::result_type
TypeInferrer::operator()(const Tree::HashSymbol& e)
{
  throw "Hash symbol appearing in type inference";
}

TypeInferrer::result_type
TypeInferrer::operator()(const Tree::HostOpExpr& e)
{

}

TypeInferrer::result_type
TypeInferrer::operator()(const Tree::ParenExpr& e)
{
  //not sure if these will even appear, but if they do, it's simply the type
  //of the body
  return apply_visitor(*this, e.e);
}

TypeInferrer::result_type
TypeInferrer::operator()(const Tree::UnaryOpExpr& e)
{
  throw "UnaryOpExpr in type inference";
}

TypeInferrer::result_type
TypeInferrer::operator()(const Tree::BinaryOpExpr& e)
{
  throw "BinaryOpExpr in type inference";
}

TypeInferrer::result_type
TypeInferrer::operator()(const Tree::MakeIntenExpr& e)
{
  auto t_0 = apply_visitor(*this, e.expr);

  auto alpha = fresh();

  auto& C = std::get<2>(t_0);

  C.add_to_closure(Constraint{TypeIntension{std::get<1>(t_0)}, alpha});

  return std::make_tuple(std::get<0>(t_0), alpha, C);
}

TypeInferrer::result_type
TypeInferrer::operator()(const Tree::EvalIntenExpr& e)
{
  auto t_0 = apply_visitor(*this, e.expr);

  auto& C = std::get<2>(t_0);

  auto alpha = fresh();
  auto gamma = fresh();

  C.add_to_closure(Constraint{std::get<1>(t_0), TypeIntension{alpha}});
  C.add_to_closure(Constraint{alpha, gamma});

  return std::make_tuple(std::get<0>(t_0), gamma, C);
}

TypeInferrer::result_type
TypeInferrer::operator()(const Tree::IfExpr& e)
{
  ConstraintGraph C;
  TypeContext A;

  auto cond_type = apply_visitor(*this, e.condition);

  A.join(std::get<0>(cond_type));

  auto then_type = apply_visitor(*this, e.then);

  A.join(std::get<0>(then_type));

  auto alpha = fresh();
  auto beta = fresh();

  C.make_union(std::get<2>(cond_type));
  C.make_union(std::get<2>(then_type));

  //make a boolean less than type var
  C.add_to_closure(Constraint{beta, 
    TypeAtomic{m_system.getTypeIndex(U"bool")}});

  C.add_to_closure(Constraint{std::get<1>(then_type), alpha});

  for (const auto& p : e.else_ifs)
  {
    auto elseif_cond_type = apply_visitor(*this, p.first);
    auto elseif_then_type = apply_visitor(*this, p.second);

    A.join(std::get<0>(elseif_cond_type));
    A.join(std::get<0>(elseif_then_type));

    C.make_union(std::get<2>(elseif_cond_type));
    C.make_union(std::get<2>(elseif_then_type));

    //each condition is less than boolean
    C.add_to_closure(Constraint{std::get<1>(elseif_cond_type), beta});

    //each result has some common upper bound
    C.add_to_closure(Constraint{std::get<1>(elseif_then_type), alpha});
  }

  auto else_type = apply_visitor(*this, e.else_);

  A.join(std::get<0>(else_type));
  
  C.make_union(std::get<2>(else_type));

  C.add_to_closure(Constraint{std::get<1>(else_type), alpha});

  return std::make_tuple(A, alpha, C);
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
  throw "cbn abstraction in type inference";
}

TypeInferrer::result_type
TypeInferrer::operator()(const Tree::BaseAbstractionExpr& e)
{
  auto t_0 = apply_visitor(*this, e.body);
  auto& A = std::get<0>(t_0);
  auto& C = std::get<2>(t_0);

  std::vector<Type> lhs;

  //remove the dimensions from the context and build a type
  for (auto d : e.dims)
  {
    lhs.push_back(A.lookup(d));
    A.remove(d);
  }

  auto alpha = fresh();

  C.add_to_closure(Constraint{TypeBase{lhs, std::get<1>(t_0)}, alpha});

  return std::make_tuple(A, alpha, C);
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
  throw "cbn application in type inference";
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
