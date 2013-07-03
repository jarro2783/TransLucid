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

#include <tl/output.hpp>

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

  c.add_to_closure(Constraint{std::forward<T>(v), t});

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

}

TypeInferrer::result_type
TypeInferrer::operator()(const Tree::ConditionalBestfitExpr& e)
{

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

      iter = m_rewrites.insert
        (std::make_pair(vars, CanoniseReplaced{gamma, lambda})).first;

      m_queue.push(*iter);
    }

    return iter->second;
  }

  private:
  CanoniseRewrites m_rewrites;
  RewriteQueue m_queue;
  FreshTypeVars m_fresh;
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
      apply_visitor(canon, glb, TagPositive()));

    //if there exists a beta in S < a, set gamma < a
    //if there exists a beta in S > a, set a < gamma
    C.rewrite_lub(current.second.gamma, current.first);
  }

  return std::make_tuple(context, typecanon, C);
}

TypeScheme
garbage_collect(const TypeScheme& t)
{
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

  while (pos.size() != currentPos.size() && neg.size() != currentNeg.size())
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

  auto C = std::get<2>(t);
  C.collect(pos, neg);

  return std::make_tuple(std::get<0>(t), std::get<1>(t), C);
}

}
}
