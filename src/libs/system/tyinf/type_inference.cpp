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
#include <tl/tyinf/type_rename.hpp>

#include <tl/types/boolean.hpp>
#include <tl/types/char.hpp>
#include <tl/types/dimension.hpp>
#include <tl/types/function.hpp>
#include <tl/types/intmp.hpp>
#include <tl/types/string.hpp>
#include <tl/types/special.hpp>

#include <tl/output.hpp>

#include <tl/system.hpp>

namespace TransLucid
{
namespace TypeInference
{

namespace
{

TypeScheme
make_host_op_type(const std::vector<type_index>& funtype, FreshTypeVars& fresh)
{
  using namespace TypeInference;

  ConstraintGraph C;

  if (funtype.size() == 0)
  {
    return std::make_tuple(TypeContext(), TypeTop(), ConstraintGraph());
  }
  else if (funtype.size() == 1)
  {
    auto t = fresh.fresh();
    //constants, but specified as a no arg base function
    C.add_to_closure(Constraint{TypeAtomic{type_index_names[funtype[0]]}, 
      t});

    return std::make_tuple(TypeContext(), t, C);
  }

  //construct a base function type
  //(x_0, ... x_{n-1}) -> x_n where x_i in funtype

  std::vector<Type> args;

  auto last = funtype.end();
  --last;

  auto iter = funtype.begin();
  while (iter != last)
  {
    auto v = fresh.fresh();
    args.push_back(v);

    C.add_to_closure(Constraint{v, TypeAtomic{type_index_names[*iter], *iter}});

    ++iter;
  }

  auto result = fresh.fresh();
  C.add_to_closure(Constraint{TypeAtomic{type_index_names[*last], *last}, 
    result});

  auto t = fresh.fresh();
  C.add_to_closure(Constraint{TypeBase{args, result}, t});

  return std::make_tuple(TypeContext(), t, C);
}

}

template <typename T>
TypeInferrer::result_type
TypeInferrer::make_constant(T&& v)
{
  TypeVariable t = fresh();

  ConstraintGraph c;

  c.add_to_closure(Constraint{std::forward<T>(v), t});

  return std::make_tuple(TypeContext(), t, c);
}

void
TypeInferrer::infer_system(const std::set<u32string>& ids)
{
  indecl = true;

  //build up C and A as we go
  TypeContext A;
  ConstraintGraph C;

  std::map<u32string, Type> types;

  std::cout << "infer_system" << std::endl;
  for (const auto& x : ids)
  {
    auto e = m_system.getIdentifierTree(x);

    auto t = apply_visitor(*this, e);

    A.join(std::get<0>(t));
    C.make_union(std::get<2>(t));

    types.insert(std::make_pair(x, std::get<1>(t)));
    
    std::cout << x << " : " << print_type(std::get<1>(t), m_system) 
      << std::endl;
  }

  for (const auto& xt : types)
  {
    //each x is allocated an alpha and a gamma type variable
    auto alpha = fresh();
    auto gamma = fresh();

    //then we link up the type from the context to the return types
    C.add_to_closure(Constraint{xt.second, alpha});
    C.add_to_closure(Constraint{alpha, gamma});
    C.add_to_closure(Constraint{gamma, A.lookup(xt.first)});
  }

  std::cout << C.print(m_system) << std::endl;
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
  if (indecl)
  {
    auto alpha = fresh();
    auto gamma = fresh();

    TypeContext A;

    A.add(e.text, alpha);

    ConstraintGraph C;
    C.add_to_closure(Constraint{alpha, gamma});

    return std::make_tuple(A, gamma, C);
  }
}

TypeInferrer::result_type
TypeInferrer::operator()(const Tree::HashSymbol& e)
{
  throw "Hash symbol appearing in type inference";
}

TypeInferrer::result_type
TypeInferrer::operator()(const Tree::HostOpExpr& e)
{
  auto f = m_system.lookupBaseFunction(e.name);

  if (f == nullptr)
  {
    auto t = fresh();
    return std::make_tuple(TypeContext(), t, ConstraintGraph());
  }
  else
  {
    auto t = make_host_op_type(f->type(), m_freshVars);
    return t;
  }
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
  C.add_to_closure(Constraint{beta, makeAtomic(m_system, U"bool")});
  C.add_to_closure(Constraint{std::get<1>(cond_type), beta});

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
  //at the moment we only handle #.d

  const Tree::DimensionExpr* dim = get<Tree::DimensionExpr>(&e.e);

  if (dim == nullptr)
  {
    throw "Invalid #.E";
  }
  else
  {
    TypeContext A;
    ConstraintGraph C;

    auto alpha = fresh();
    auto gamma = fresh();

    A.add(dim->dim, alpha);
    C.add_to_closure(Constraint{alpha, gamma});

    return std::make_tuple(A, gamma, C);
  }
}

TypeInferrer::result_type
TypeInferrer::operator()(const Tree::RegionExpr& e)
{
  auto alpha = fresh();
  ConstraintGraph C;
  C.add_to_closure(Constraint{TypeRegion(), alpha});

  return std::make_tuple(TypeContext(), alpha, C);
}

TypeInferrer::result_type
TypeInferrer::operator()(const Tree::TupleExpr& e)
{
  auto alpha = fresh();
  ConstraintGraph C;
  C.add_to_closure(Constraint{TypeTuple(), alpha});

  return std::make_tuple(TypeContext(), alpha, C);
}

TypeInferrer::result_type
TypeInferrer::operator()(const Tree::AtExpr& e)
{
  auto t_1 = apply_visitor(*this, e.rhs);
  auto t_0 = apply_visitor(*this, e.lhs);

  auto& C = std::get<2>(t_0);
  C.make_union(std::get<2>(t_1));
  C.add_to_closure(Constraint{std::get<1>(t_1), TypeTuple()});

  auto& A = std::get<0>(t_0);
  A.join(std::get<0>(t_1));

  return std::make_tuple(A, std::get<1>(t_0), C);
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
  lhs.reserve(e.dims.size());

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
  auto t_0 = apply_visitor(*this, e.name);

  TypeContext A = std::get<0>(t_0);
  ConstraintGraph C = std::get<2>(t_0);

  std::vector<Type> args;
  args.reserve(e.args.size());

  for (auto arg : e.args)
  {
    auto t_i = apply_visitor(*this, arg);
    C.make_union(std::get<2>(t_i));
    A.join(std::get<0>(t_i));

    args.push_back(std::get<1>(t_i));
  }

  auto alpha = fresh();
  auto gamma = fresh();

  C.add_to_closure(Constraint{std::get<1>(t_0),
    TypeBase{args, alpha}});
  C.add_to_closure(Constraint{alpha, gamma});

  return std::make_tuple(A, gamma, C);
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
  auto t_0 = apply_visitor(*this, e.e);

  auto& A = std::get<0>(t_0);
  auto& C = std::get<2>(t_0);

  for (const auto& d : e.dims)
  {
    auto t = apply_visitor(*this, d.second);

    A.join(std::get<0>(t));
    C.make_union(std::get<2>(t));
  }

  return std::make_tuple(A, std::get<1>(t_0), C);
}

TypeInferrer::result_type
TypeInferrer::operator()(const Tree::ConditionalBestfitExpr& e)
{
  TypeContext A;
  ConstraintGraph C;

  auto alpha = fresh();

  for (const auto& d : e.declarations)
  {
    auto t_0 = apply_visitor(*this, std::get<1>(d));
    auto t_1 = apply_visitor(*this, std::get<2>(d));
    auto t_2 = apply_visitor(*this, std::get<3>(d));

    if (!variant_is_type<TypeNothing>(std::get<1>(t_0)))
    {
      A.join(std::get<0>(t_0));
      C.make_union(std::get<2>(t_0));

      C.add_to_closure(Constraint{std::get<1>(t_0), TypeRegion()});
    }
    if (!variant_is_type<TypeNothing>(std::get<1>(t_1)))
    {
      A.join(std::get<0>(t_1));
      C.make_union(std::get<2>(t_1));

      C.add_to_closure(Constraint{std::get<1>(t_0), 
        makeAtomic(m_system, U"bool")
      });
    }

    A.join(std::get<0>(t_2));
    C.make_union(std::get<2>(t_2));

    C.add_to_closure(Constraint{std::get<1>(t_2), alpha});
  }

  return std::make_tuple(A, alpha, C);
}

TypeAtomic
makeAtomic(TypeRegistry& reg, const u32string& name)
{
  return TypeAtomic{name, reg.getTypeIndex(name)};
}

namespace
{

struct CanoniseReplaced
{
  TypeVariable gamma;
  TypeVariable lambda;
};

typedef std::map<VarSet, CanoniseReplaced> CanoniseRewrites;
typedef std::queue<CanoniseRewrites::value_type> RewriteQueue;

struct TagNegative {};
struct TagPositive {};

class CanoniseVars
{
  public:

  CanoniseVars(RewriteQueue& q, FreshTypeVars& fresh)
  : m_queue(q), m_fresh(fresh)
  {
  }

  CanoniseReplaced
  lookup(const VarSet& vars)
  {
    auto iter = m_rewrites.find(vars);

    if (iter == m_rewrites.end())
    {
      //generate fresh gamma and lambda
      //add to queue to generate new constraints

      auto gamma = m_fresh.fresh();
      auto lambda = m_fresh.fresh();

      std::cout << "S = {";
      print_container(std::cout, vars);
      std::cout << "} |-> (" << gamma << ", " << lambda << ")" << std::endl;

      iter = m_rewrites.insert
        (std::make_pair(vars, CanoniseReplaced{gamma, lambda})).first;

      m_queue.push(*iter);
    }

    return iter->second;
  }

  const CanoniseRewrites&
  rewrites() const
  {
    return m_rewrites;
  }

  private:
  CanoniseRewrites m_rewrites;
  RewriteQueue& m_queue;
  FreshTypeVars& m_fresh;
};

class Canonise
{
  public:

  typedef Type result_type;

  Canonise(CanoniseVars& vars)
  : m_vars(vars)
  {
  }

  //for all the types that do not get rewritten and are the same regardless
  //of polarity
  template <typename Type, typename Tag>
  Type
  operator()(const Type& type, Tag tag)
  {
    return type;
  }

  Type
  operator()(const TypeLUB& lub, TagPositive)
  {
    return m_vars.lookup(lub.vars).lambda;
  }

  Type
  operator()(const TypeGLB& glb, TagNegative)
  {
    return m_vars.lookup(glb.vars).gamma;
  }

  Type
  operator()(const TypeIntension& inten, TagPositive)
  {
    return TypeIntension{apply_visitor(*this, inten.body, TagPositive())};
  }

  Type
  operator()(const TypeIntension& inten, TagNegative)
  {
    return TypeIntension{apply_visitor(*this, inten.body, TagNegative())};
  }

  Type
  operator()(const TypeCBV& cbv, TagPositive)
  {
    return TypeCBV
    {
      apply_visitor(*this, cbv.lhs, TagNegative()),
      apply_visitor(*this, cbv.rhs, TagPositive())
    };
  }

  Type
  operator()(const TypeCBV& cbv, TagNegative)
  {
    return TypeCBV
    {
      apply_visitor(*this, cbv.lhs, TagPositive()),
      apply_visitor(*this, cbv.rhs, TagNegative())
    };
  }

  Type
  operator()(const TypeBase& base, TagPositive)
  {
    std::vector<Type> lhs;
    lhs.reserve(base.lhs.size());

    for (auto t : base.lhs)
    {
      lhs.push_back(apply_visitor(*this, t, TagNegative()));
    }

    return TypeBase{lhs, apply_visitor(*this, base.rhs, TagPositive())};
  }

  Type
  operator()(const TypeBase& base, TagNegative)
  {
    std::vector<Type> lhs;
    lhs.reserve(base.lhs.size());

    for (auto t : base.lhs)
    {
      lhs.push_back(apply_visitor(*this, t, TagPositive()));
    }

    return TypeBase{lhs, apply_visitor(*this, base.rhs, TagNegative())};
  }

  private:
  CanoniseVars& m_vars;
};

class MarkPolarity
{
  public:

  //positive, negative
  typedef std::pair<VarSet, VarSet> result_type;

  template <typename T, typename Polarity>
  result_type
  operator()(const T& t, Polarity)
  {
    return result_type();
  }

  result_type
  operator()(TypeVariable v, TagPositive)
  {
    return {{v}, {}};
  }

  result_type
  operator()(TypeVariable v, TagNegative)
  {
    return {{}, {v}};
  }

  result_type
  operator()(const TypeCBV& cbv, TagPositive)
  {
    auto lhs = apply_visitor(*this, cbv.lhs, TagNegative());
    auto rhs = apply_visitor(*this, cbv.rhs, TagPositive());

    VarSet pos = lhs.first;
    pos.insert(rhs.first.begin(), rhs.first.end());

    VarSet neg = lhs.second;
    neg.insert(rhs.second.begin(), rhs.second.end());

    return {pos, neg};
  }

  result_type
  operator()(const TypeCBV& cbv, TagNegative)
  {
    auto lhs = apply_visitor(*this, cbv.lhs, TagPositive());
    auto rhs = apply_visitor(*this, cbv.rhs, TagNegative());
    
    VarSet pos = lhs.first;
    pos.insert(rhs.first.begin(), rhs.first.end());

    VarSet neg = lhs.second;
    neg.insert(rhs.second.begin(), rhs.second.end());

    return {pos, neg};
  }

  result_type
  operator()(const TypeBase& base, TagPositive)
  {
    VarSet pos;
    VarSet neg;

    for (const auto& t : base.lhs)
    {
      auto polarity = apply_visitor(*this, t, TagNegative());
      pos.insert(polarity.first.begin(), polarity.first.end());
      neg.insert(polarity.second.begin(), polarity.second.end());
    }

    auto rhs = apply_visitor(*this, base.rhs, TagPositive());
    
    pos.insert(rhs.first.begin(), rhs.first.end());
    neg.insert(rhs.second.begin(), rhs.second.end());

    return {pos, neg};
  }

  result_type
  operator()(const TypeBase& base, TagNegative)
  {
    VarSet pos;
    VarSet neg;

    for (const auto& t : base.lhs)
    {
      auto polarity = apply_visitor(*this, t, TagPositive());
      pos.insert(polarity.first.begin(), polarity.first.end());
      neg.insert(polarity.second.begin(), polarity.second.end());
    }

    auto rhs = apply_visitor(*this, base.rhs, TagNegative());
    
    pos.insert(rhs.first.begin(), rhs.first.end());
    neg.insert(rhs.second.begin(), rhs.second.end());

    return {pos, neg};
  }

  result_type
  operator()(const TypeIntension& inten, TagPositive)
  {
    return apply_visitor(*this, inten.body, TagPositive());
  }

  result_type
  operator()(const TypeIntension& inten, TagNegative)
  {
    return apply_visitor(*this, inten.body, TagNegative());
  }

};

struct VarInSet
{
  VarInSet(const VarSet& v)
  : m_v(v)
  {
  }

  template <typename T>
  bool
  operator()(const T& t)
  {
    return m_v.find(t.first) != m_v.end();
  }

  const VarSet& m_v;
};

enum class HeadOrder
{
  TYPE_NOTHING,
  TYPE_TOP,
  TYPE_BOT,
  TYPE_VARIABLE,
  CONSTANT,
  TYPE_ATOMIC,
  TYPE_REGION,
  TYPE_TUPLE,
  TYPE_GLB,
  TYPE_LUB,
  TYPE_INTENSION,
  TYPE_CBV,
  TYPE_BASE
};

struct GetTypeOrder
{
  typedef HeadOrder result_type;

  result_type
  operator()(TypeNothing const&)
  {
    return HeadOrder::TYPE_NOTHING;
  }

  result_type
  operator()(TypeTop const&)
  {
    return HeadOrder::TYPE_TOP;
  }

  result_type
  operator()(TypeBot const&)
  {
    return HeadOrder::TYPE_BOT;
  }

  result_type
  operator()(TypeVariable const&)
  {
    return HeadOrder::TYPE_VARIABLE;
  }

  result_type
  operator()(Constant const&)
  {
    return HeadOrder::CONSTANT;
  }

  result_type
  operator()(TypeAtomic const&)
  {
    return HeadOrder::TYPE_ATOMIC;
  }

  result_type
  operator()(TypeRegion const&)
  {
    return HeadOrder::TYPE_REGION;
  }

  result_type
  operator()(TypeTuple const&)
  {
    return HeadOrder::TYPE_TUPLE;
  }

  result_type
  operator()(TypeGLB const&)
  {
    return HeadOrder::TYPE_GLB;
  }

  result_type
  operator()(TypeLUB const&)
  {
    return HeadOrder::TYPE_LUB;
  }

  result_type
  operator()(TypeIntension const&)
  {
    return HeadOrder::TYPE_INTENSION;
  }

  result_type
  operator()(TypeCBV const&)
  {
    return HeadOrder::TYPE_CBV;
  }

  result_type
  operator()(TypeBase const&)
  {
    return HeadOrder::TYPE_BASE;
  }
};

struct MinimiseCompare
{
  bool
  operator()(TypeVariable a, TypeVariable b)
  {
    //sort by predecessor first
    auto preda = C.predecessor(a);
    auto predb = C.predecessor(b);

    if (std::lexicographical_compare(preda.begin(), preda.end(),
          predb.begin(), predb.end()))
    {
      return true;
    }
    else if (preda != predb)
    {
      return false;
    }

    //then by successor
    auto succa = C.successor(a);
    auto succb = C.successor(b);

    if (std::lexicographical_compare(succa.begin(), succa.end(),
          succb.begin(), succb.end()))
    {
      return true;
    }
    else if (succa != succb)
    {
      return false;
    }

    //sort negatives less than positives
    bool aNeg = neg.find(a) != neg.end();
    bool bNeg = neg.find(b) != neg.end();
    if (aNeg && !bNeg)
    {
      return true;
    }

    if (!aNeg && bNeg)
    {
      return false;
    }

    //otherwise they are equal, so continue
    GetTypeOrder headOrder;

    //then sort by head of lower bound
    HeadOrder aLowerHead = apply_visitor(headOrder, C.lower(a));
    HeadOrder bLowerHead = apply_visitor(headOrder, C.lower(b));

    if (aLowerHead < bLowerHead)
    {
      return true;
    }
    else if (bLowerHead < aLowerHead)
    {
      return false;
    }

    //then by head of upper bound
    HeadOrder aUpperHead = apply_visitor(headOrder, C.upper(a));
    HeadOrder bUpperHead = apply_visitor(headOrder, C.upper(b));

    if (aUpperHead < bUpperHead)
    {
      return true;
    }
    else if (bUpperHead < aUpperHead)
    {
      return false;
    }

    //if all else fails they are equal
    return false;
  }

  bool
  equal(TypeVariable a, TypeVariable b)
  {
    //check predecessor first
    auto preda = C.predecessor(a);
    auto predb = C.predecessor(b);

    if (preda != predb)
    {
      return false;
    }

    //then successor
    auto succa = C.successor(a);
    auto succb = C.successor(b);
    
    if (succa != succb)
    {
      return false;
    }

    //they both have to be positive or negative
    if ((neg.find(a) != neg.end())
        != (neg.find(b) != neg.end()))
    {
      return false;
    }

    GetTypeOrder headOrder;

    if (apply_visitor(headOrder, C.lower(a)) != 
        apply_visitor(headOrder, C.lower(b)))
    {
      return false;
    }

    if (apply_visitor(headOrder, C.upper(a)) != 
        apply_visitor(headOrder, C.upper(b)))
    {
      return false;
    }
    
    return true;
  }

  const VarSet& neg;
  const ConstraintGraph& C;
};

struct MinimiseBlock
{
  std::set<TypeVariable> vars;
};

struct MinimiseMapper
{
  void
  addToMappings(TypeVariable s, std::vector<std::vector<TypeVariable>>& m)
  {
  }

  const ConstraintGraph& C;
};

//a mapping from type variables to their inverse for each particular function
typedef std::map<TypeVariable, std::map<size_t, std::set<TypeVariable>>> 
  InverseSet;

//returns the set and the number of functions
std::pair<InverseSet, size_t>
generateInverse(const ConstraintGraph& C)
{
  MinimiseMapper m{C};

  InverseSet inverse;

  return std::make_pair(inverse, 0);
}

}

TypeScheme
canonise(const TypeScheme& t, FreshTypeVars& fresh)
{
  RewriteQueue queue;
  CanoniseVars vars(queue, fresh);
  Canonise canon(vars);

  //rewrite A as negative, t as positive, all the lower and upper bounds in
  //C as positive and negative respectively, then add
  //constraints until the queue is empty

  auto typecanon = apply_visitor(canon, std::get<1>(t), TagPositive());
  auto context = TypeContext::rewrite(std::get<0>(t), 
    std::bind(visitor_applier(), std::ref(canon), std::placeholders::_1,
      TagNegative())
  );

  //rewrite everything in C
  auto C = std::get<2>(t);
  C.rewrite_bounds
  (
    std::bind(visitor_applier(), std::ref(canon), std::placeholders::_1,
      TagPositive()),
    std::bind(visitor_applier(), std::ref(canon), std::placeholders::_1,
      TagNegative())
  );

  while (!queue.empty())
  {
    auto current = queue.front();
    queue.pop();

    //upper bound of gamma is the negative rewrite of the glb of all the 
    //upper bounds of the variables in S

    //lower bound of lambda is the positive rewrite of the lub of all the
    //lower bounds of the variables in S
    Type glb = TypeTop();
    Type lub = TypeBot();

    for (auto v : current.first)
    {
      glb = construct_glb(glb, C.upper(v));
      lub = construct_lub(lub, C.lower(v));
    }

    C.setUpper(current.second.gamma, apply_visitor(canon, glb, TagNegative()));
    C.setLower(current.second.lambda, 
      apply_visitor(canon, lub, TagPositive()));

    //if there exists a beta in S such that beta < a, set gamma < a
    //if there exists a beta in S such that a < beta, set a < lambda
    C.rewrite_less(current.second.gamma, current.first);
    C.rewrite_greater(current.second.lambda, current.first);
  }

  //for everything rewritten, gamma_S < lambda_T if 
  //something in S < something in T
  auto& rewrites = vars.rewrites();
  auto itera = rewrites.begin();
  auto iterb = itera;

  while (itera != rewrites.end())
  {
    //iterb = itera;
    //if (iterb != rewrites.end())
    //{
      ++iterb;
    //}

    while (iterb != rewrites.end())
    {
      C.rewrite_lub_glb(itera->second.gamma, itera->first, 
        iterb->second.lambda, iterb->first);
      C.rewrite_lub_glb(iterb->second.gamma, iterb->first, 
        itera->second.lambda, itera->first);
      ++iterb;
    }
    ++itera;
  }

  return std::make_tuple(context, typecanon, C);
}

//(neg, pos)
std::pair<VarSet, VarSet>
polarity(const TypeScheme& t)
{
  
  MarkPolarity mark;

  VarSet pos;
  VarSet neg;

  auto tpol = apply_visitor(mark, std::get<1>(t), TagPositive());

  pos = tpol.first;
  neg = tpol.second;

  auto gatherneg = [&] (const Type& t, VarSet& p, VarSet& n) -> void
    {
      auto result = apply_visitor(mark, t, TagNegative());

      p.insert(result.first.begin(), result.first.end());
      n.insert(result.second.begin(), result.second.end());
    };

  auto gatherpos = [&] (const Type& t, VarSet& p, VarSet& n) -> void
    {
      auto result = apply_visitor(mark, t, TagPositive());

      pos.insert(result.first.begin(), result.first.end());
      neg.insert(result.second.begin(), result.second.end());
    };

  std::get<0>(t).for_each(
    std::bind(gatherneg, std::placeholders::_1, std::ref(pos), std::ref(neg)));

  VarSet currentPos;
  VarSet currentNeg;

  while (pos.size() != currentPos.size() || neg.size() != currentNeg.size())
  {
    currentPos = pos;
    currentNeg = neg;

    std::get<2>(t).for_each_lower_if(
      std::bind(gatherpos, 
        std::placeholders::_1,
        std::ref(pos),
        std::ref(neg)
      ),
      VarInSet(currentPos)
    );

    std::get<2>(t).for_each_upper_if(
      std::bind(gatherneg, 
        std::placeholders::_1,
        std::ref(pos),
        std::ref(neg)
      ),
      VarInSet(currentNeg)
    );
  }

  std::cout << "positive variables" << std::endl;
  print_container(std::cout, pos);
  std::cout << std::endl;
  std::cout << "negative variables" << std::endl;
  print_container(std::cout, neg);
  std::cout << std::endl;

  return std::make_pair(neg, pos);
}

TypeScheme
garbage_collect(const TypeScheme& t)
{
  auto p = polarity(t);

  auto C = std::get<2>(t);
  C.collect(p.first, p.second);

  return std::make_tuple(std::get<0>(t), std::get<1>(t), C);
}

TypeScheme
minimise(const TypeScheme& t)
{
  //finite state automata minimisation, but with the "transition functions" 
  //being that the components of the upper and lower bounds fall within
  //the same block
  auto& C = std::get<2>(t);

  auto p = polarity(t);

  auto vars = C.domain();

  //if there is only zero or one variable our job is done
  if (vars.size() <= 1)
  {
    return t;
  }

  MinimiseCompare compare{p.first, C};

  auto inverseResult = generateInverse(C);
  InverseSet& inverse = inverseResult.first;
  auto functions = inverseResult.second;

  //first sort the type variables to get the initial partition
  std::sort(vars.begin(), vars.end(), compare);

  std::cout << "sorted vars:" << std::endl;
  std::copy(vars.begin(), vars.end(), 
    std::ostream_iterator<TypeVariable>(std::cout, ", "));
  std::cout << std::endl;

  //divide into the initial partition of acceptable blocks
  std::map<size_t, MinimiseBlock> blocks;
  std::map<TypeVariable, size_t> inverseBlocks;

  size_t b = 0;
  int maxDist = 0;
  size_t maxBlock = 0;
  auto iter = vars.begin();
  auto start = iter;

  //there are at least two variables to deal with, so ++iter is defined
  ++iter;

  while (iter != vars.end())
  {
    if (!compare.equal(*start, *iter))
    {
      auto dist = iter - start;

      //make the block
      blocks.insert(std::make_pair(b, MinimiseBlock{{start, iter}}));

      //record which block each variable is in
      while (start != iter)
      {
        inverseBlocks.insert(std::make_pair(*start, b));
        ++start;
      }

      ++b;

      //keep track of the biggest block
      if (dist > maxDist)
      {
        maxBlock = b;
        maxDist = dist;
      }
    }

    ++iter;
  }

  //put the last range in
  if (start - iter > 1)
  {
    blocks.insert(std::make_pair(b, MinimiseBlock{{start, iter}}));
  }

  //now we try to reduce the size of the blocks we already have
  //the blocks mapped to the functions to split by
  std::map<size_t, std::set<size_t>> toSplit;

  //also keep a list of iterators into the splits so that we can keep a queue
  //easily
  std::list<
    std::pair<decltype(toSplit)::iterator,
      decltype(toSplit)::mapped_type::iterator>>
  splitQueue;

  //initially partition by all functions
  std::set<size_t> funpart;
  for (size_t f = 0; f != functions; ++f)
  {
    funpart.insert(f);
  }
  

  //partition with respect to all but the biggest and every seen variable
  for (const auto& block : blocks)
  {
    if (block.first != maxBlock)
    {
      auto result = toSplit.insert(std::make_pair(block.first, funpart)).first;

      auto iter = result->second.begin();
      while (iter != result->second.end())
      {
        splitQueue.push_back(std::make_pair(result, iter));
        ++iter;
      }
    }
  }

  MinimiseMapper mapper{C};

  while (!splitQueue.empty())
  {
    //pick something out of the list
    auto currentSplit = *splitQueue.begin();

    //remove the first one from the list
    splitQueue.pop_front();
    currentSplit.first->second.erase(currentSplit.second);

    //get the inverse of the current block and symbol
    std::set<TypeVariable> currentInverse;
    for (auto s : blocks.find(currentSplit.first->first)->second.vars)
    {
      auto inverseIter = inverse.find(s);

      auto inverseFunIter = inverseIter->second.find(*currentSplit.second);

      currentInverse.insert(inverseFunIter->second.begin(), 
        inverseFunIter->second.end());
    }

    //gather the blocks in the inverse
    std::set<size_t> seenBlocks;
    for (auto s : currentInverse)
    {
      seenBlocks.insert(inverseBlocks.find(s)->second);
    }

    //for each s in the inverse, if all of the states in its block are
    //mapped then do nothing, if they are not all mapped then move s to
    //its block's twin

    //start by caching whether the seen blocks map all
    for (auto b : seenBlocks)
    {
      auto currentBlock = blocks.find(b);

      //for the current block B for each element s of B, all the mappings 
      //for each function f(s)
      std::vector<std::vector<TypeVariable>> mappings;
      for (auto s : currentBlock->second.vars)
      {
        mapper.addToMappings(s, mappings);
      }
    }
  }

  return t;
}

}
}
