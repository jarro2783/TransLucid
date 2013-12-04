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

#include <tl/ast.hpp>
#include <tl/fixed_indexes.hpp>
#include <tl/free_variables.hpp>
#include <tl/output.hpp>
#include <tl/workshop_builder.hpp>

#include <tl/tyinf/free_type_variable.hpp>
#include <tl/tyinf/type_error.hpp>
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

#include <stack>

//define this to turn off type simplification
//#define TL_TYINF_NO_SIMPLIFY

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
    C.add_to_closure(Constraint{TypeAtomic{type_index_names[funtype[0]], 
      funtype[0]}, t});

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

TypeScheme
TypeInferrer::simplify(TypeScheme t)
{
  //std::get<0>(t).fix_context(std::get<2>(t));
  auto S = 
  #ifndef TL_TYINF_NO_SIMPLIFY
    minimise(
      garbage_collect(
        canonise(
  #endif
          t
  #ifndef TL_TYINF_NO_SIMPLIFY
          , m_freshVars
        )
      )
    )
  #endif
  ;

  std::get<0>(S).instantiate_parameters(std::get<2>(S));
  #ifndef TL_TYINF_NO_SIMPLIFY
  //now, to sort out the TL context, rerun simplification
  S = minimise(garbage_collect(canonise(S, m_freshVars)));
  #endif

  return S;
}

void
TypeInferrer::infer_system(const std::set<u32string>& ids)
{
  indecl = true;

  std::cout << "infer_system" << std::endl;

  std::cout << "inferring types of : ";
  print_container(std::cout, ids);
  std::cout << std::endl;

  auto recursion_groups = generate_recurse_groups(ids);

  const u32string* currentId = nullptr;

  try
  {

    for (const auto& group : recursion_groups)
    {
      //build up C and A as we go
      TypeContext A;
      ConstraintGraph C;
      std::map<u32string, Type> types;

      for (const auto& x : group)
      {
        //we can cheat sometimes and provide appropriate types when we
        //know that they can't be inferred properly

        decltype(types.begin()) typeInserted;

        auto iter = m_environment.find(x);
        if (iter != m_environment.end())
        {
          auto S = rename_scheme(iter->second, m_freshVars);
          //Rename rename(m_freshVars);
          //auto S = rename.rename(iter->second);

          A.join(std::get<0>(S));
          C.make_union(std::get<2>(S));
          typeInserted = types.insert(std::make_pair(x, std::get<1>(S))).first;
        }
        else
        {
          currentId = &x;
          auto e = m_system.getIdentifierTree(x);

          std::cout << "inferring : " << x << std::endl;

          auto t = apply_visitor(*this, e);

          A.join(std::get<0>(t));
          C.make_union(std::get<2>(t));

          typeInserted = types.insert(std::make_pair(x, std::get<1>(t))).first;
        }

        std::cout << x << " : " << 
          print_type(typeInserted->second, m_system) 
          << std::endl;
      }
        
      if (types.size() > 1)
      {
        for (const auto& xt : types)
        {
          currentId = &xt.first;
          //each x is allocated an alpha and a gamma type variable
          auto alpha = fresh();
          auto gamma = fresh();

          //then we link up the type from the context to the return types
          C.add_to_closure(Constraint{xt.second, alpha});
          C.add_to_closure(Constraint{alpha, gamma});
          C.add_to_closure(Constraint{gamma, A.lookup(xt.first)});
        }
      }

      //remove each var from the context
      for (const auto& xt : types)
      {
        A.remove(xt.first);
      }

      for (const auto& xt : types)
      {
        auto S =  simplify(
        #if 0
        #ifndef TL_TYINF_NO_SIMPLIFY
          minimise(
            garbage_collect(
              canonise(
        #endif
        #endif
                std::make_tuple(A, xt.second, C)
        #if 0
        #ifndef TL_TYINF_NO_SIMPLIFY
                , m_freshVars
              )
            )
          )
        #endif
        #endif
        )
        ;

        std::cout << std::get<2>(S).print(m_system) << "\n";
        std::cout << "\nContext: ";
        std::cout << std::get<0>(S).print_context(m_system) << std::endl;
        #if 0 
        for (const auto& d : std::get<0>(S).getDimensions())
        {
          std::cout << "(" << d.first << ", (" << 
            print_type(d.second.first, m_system) << ", " <<
            print_type(d.second.second, m_system)
            << ")) ";
        }
        #endif

        m_environment[xt.first] = S;
      }
    }
  } 
  catch (...)
  {
    std::cerr << "Error checking type of " << *currentId << std::endl;
    throw;
  }
}

std::vector<std::vector<u32string>>
TypeInferrer::generate_recurse_groups(const std::set<u32string>& ids)
{
  FreeVariables free;

  //get the free variables in each variable
  std::map<u32string, std::set<u32string>> freeVariables;

  std::vector<u32string> newVars;
  std::vector<u32string> currentRound(ids.begin(), ids.end());

  while (!currentRound.empty())
  {
    for (const auto& v : currentRound)
    {
      auto result = freeVariables.insert(
        std::make_pair(v, free.findFree(m_system.getIdentifierTree(v))));

      for (const auto& f : result.first->second)
      {
        if (freeVariables.find(f) == freeVariables.end())
        {
          newVars.push_back(f);
        }
      }
    }

    currentRound = newVars;
    newVars.clear();
  }

  //construct a graph out of the free variables, but make the graph with
  //integers. So we need to keep a mapping from integers to their strings
  std::map<u32string, size_t> stringIndices;
  std::vector<u32string> indexToString;

  auto updateIndex = [&] (const u32string& s) -> size_t
    {
      size_t index = 0;
      auto iter = stringIndices.find(s);
      if (iter == stringIndices.end())
      {
        index = indexToString.size();
        indexToString.push_back(s);
        stringIndices.insert(std::make_pair(s, index));
      }
      else
      {
        index = iter->second;
      }

      return index;
    };

  //vertex list for dependency graph
  std::vector<std::vector<size_t>> depGraph(ids.size());

  for (const auto& var : freeVariables)
  {
    size_t index = updateIndex(var.first);

    //make sure that there is an entry for this identifier
    if (depGraph.size() <= index)
    {
      depGraph.resize(index+1);
    }

    for (const auto& f : var.second)
    {
      size_t fi = updateIndex(f);

      depGraph[index].push_back(fi);
    }
  }

  auto connected = generate_strongly_connected(depGraph);

  std::vector<std::vector<u32string>> groups;

  for (const auto& component : connected)
  {
    std::vector<u32string> oneGroup;
    for (const auto& v : component)
    {
      oneGroup.push_back(indexToString[v]);
    }
    groups.push_back(oneGroup);
  }

  return groups;
}

TypeContext
TypeInferrer::process_region_guard
(
  const Tree::RegionExpr& r,
  std::vector<std::pair<dimension_index, Type>>& result
)
{
  TypeContext A;

  for (const auto& e : r.entries)
  {
    if (variant_is_type<Tree::DimensionExpr>(std::get<0>(e)))
    {
      //this only works if these objects are constants, i.e., the lower bound
      //of this type is a Constant object
      auto d = get<Tree::DimensionExpr>(std::get<0>(e)).dim;
      auto rhs = apply_visitor(*this, std::get<2>(e));

      auto t = std::get<2>(rhs).lower(get<TypeVariable>(std::get<1>(rhs)));
      A.join(std::get<0>(rhs));

      std::cout << "context of rhs of " << d << "\n" << 
        std::get<0>(rhs).print_context(m_system)
        << std::endl;

      Type currentType;

      if (variant_is_type<Constant>(t))
      {
        //now we can get to work

        const auto& c = get<Constant>(t);
        switch (std::get<1>(e))
        {
          case Region::Containment::IMP:
          //this has to be an atomic type, otherwise it is a type error
          if (c.index() == TYPE_INDEX_TYPE)
          {
            auto baseType = get_constant<type_index>(c);
            currentType = TypeAtomic{m_system.getTypeName(baseType), baseType};
            result.push_back(std::make_pair(d, currentType));

            //std::cout << "dimension " << d << " has type " << baseType 
            //  << std::endl;
          }
          else
          {
            throw RegionImpInvalid(r);
          }
          break;

          case Region::Containment::IS:
          //this one is easy, it is exactly equal to the object
          result.push_back(std::make_pair(d, c));
          //std::cout << "dimension " << d << " is a constant" << std::endl;
          break;

          case Region::Containment::IN:
          break;
        }
      }
      else if (variant_is_type<TypeAtomic>(t) && 
        get<TypeAtomic>(t).index == TYPE_INDEX_REGION)
      {
        //if we see a region, accept integers
        result.push_back(std::make_pair(d,
          TypeAtomic{U"intmp", TYPE_INDEX_INTMP}));
      }
    }
  }

  return A;
}

TypeInferrer::result_type
TypeInferrer::infer(const Tree::Expr& e)
{
  auto t = apply_visitor(*this, e);

  auto S = simplify(t);
  //std::get<0>(S).clear_context();

  return simplify(S);
}

std::pair<TypeScheme, TypeScheme>
TypeInferrer::separate_context(const TypeScheme& t)
{
  TypeScheme context = t;
  TypeScheme bare = t;

  std::get<0>(bare).clear_raw_context();
  std::get<1>(context) = TypeNothing();

  context = simplify(context);
  bare = simplify(bare);

  return std::make_pair(bare, context);
}

TypeInferrer::result_type
TypeInferrer::operator()(const Tree::nil& e)
{
  auto alpha = fresh();
  return result_type(TypeContext(), alpha, ConstraintGraph());
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
TypeInferrer::operator()(const Constant& c)
{
  return make_constant(c);
}

TypeInferrer::result_type
TypeInferrer::operator()(const Tree::LiteralExpr& e)
{
  //evaluate the literal, and that is its type
  WorkshopBuilder compile(&m_system);

  std::shared_ptr<WS> ws(compile.build_workshops(e.rewritten));

  Constant result = (*ws)(m_system.getDefaultContext());

  ConstraintGraph C;
  auto t = fresh();

  C.add_to_closure(Constraint{result, t});

  return std::make_tuple(TypeContext(), t, C);
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
  auto iter = m_environment.find(e.text);
  if (iter == m_environment.end())
  {
    auto alpha = fresh();
    auto gamma = fresh();

    TypeContext A;

    A.add(e.text, alpha);

    ConstraintGraph C;
    C.add_to_closure(Constraint{alpha, gamma});

    return std::make_tuple(A, gamma, C);
  }
  else
  {
    //rename the type scheme and return
    return rename_scheme(iter->second, m_freshVars);
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

  auto latestUpper = alpha;
  auto latestCond = get<TypeVariable>(std::get<1>(cond_type));

  C.make_union(std::get<2>(cond_type));
  C.make_union(std::get<2>(then_type));

  //make a boolean less than type var
  C.add_to_closure(Constraint{beta, makeAtomic(m_system, U"bool")});

  //the condition is less than bool
  C.add_to_closure(Constraint{std::get<1>(cond_type), beta});

  //if true is less than t_0, the then is in the type
  C.add_conditional(CondConstraint
    {
      Types::Boolean::create(true),
      latestCond,
      get<TypeVariable>(std::get<1>(then_type)), 
      alpha
    });

  for (const auto& p : e.else_ifs)
  {
    auto elseBound = fresh();

    auto elseif_cond_type = apply_visitor(*this, p.first);
    auto elseif_then_type = apply_visitor(*this, p.second);

    A.join(std::get<0>(elseif_cond_type));
    A.join(std::get<0>(elseif_then_type));

    C.make_union(std::get<2>(elseif_cond_type));
    C.make_union(std::get<2>(elseif_then_type));

    //each condition is less than boolean
    C.add_to_closure(Constraint{std::get<1>(elseif_cond_type), beta});

    //the else if type only comes into play if the condition is true and the
    //previous condition is false

    C.add_conditional(CondConstraint
      {
        Types::Boolean::create(false),
        latestCond,
        elseBound,
        latestUpper
      });

    latestCond = get<TypeVariable>(std::get<1>(elseif_cond_type));

    C.add_conditional(CondConstraint
      {
        Types::Boolean::create(true),
        latestCond,
        get<TypeVariable>(std::get<1>(elseif_then_type)),
        elseBound
      });

    latestUpper = elseBound;
  }

  auto else_type = apply_visitor(*this, e.else_);

  A.join(std::get<0>(else_type));
  
  C.make_union(std::get<2>(else_type));

  //the else type occurs if the latest condition is false (which will be
  //alpha if there were no else ifs
  C.add_conditional(CondConstraint
    {
      Types::Boolean::create(false),
      latestCond,
      get<TypeVariable>(std::get<1>(else_type)),
      latestUpper
    });

  return std::make_tuple(A, alpha, C);
}

TypeInferrer::result_type
TypeInferrer::operator()(const Tree::HashExpr& e)
{
  //#.d, #.(#.d), and #.E are all different
  //#.d means we are retrieving a parameter
  //#.(#.d) means we are looking up a passed parameter in the context
  //#.E means we are looking up a constant dimension given by an expression
  const Tree::DimensionExpr* dim = get<Tree::DimensionExpr>(&e.e);
  const Tree::HashExpr* hash = get<Tree::HashExpr>(&e.e);

  if (dim != nullptr)
  {
    TypeContext A;
    ConstraintGraph C;

    auto alpha = fresh();
    auto gamma = fresh();

    A.add(dim->dim, alpha);
    C.add_to_closure(Constraint{alpha, gamma});

    return std::make_tuple(A, gamma, C);
  }
  else if (hash != nullptr)
  {
    //is it #.(#.d) ?
    dim = get<Tree::DimensionExpr>(&hash->e);

    if (dim != nullptr)
    {
      TypeContext A;
      ConstraintGraph C;

      //variables for parameter lookup
      auto alpha = fresh();

      A.add(dim->dim, alpha);

      //the variables for the dimension query
      auto value = fresh();
      auto upper = fresh();
      auto lower = fresh();
      auto thedim = fresh();

      A.addParamDim(dim->dim, thedim, lower, upper);
      C.add_to_closure(Constraint{upper, value});
      C.add_to_closure(Constraint{alpha, thedim});

      return std::make_tuple(A, value, C);
    }
    else
    {
      //this is an error
      throw "Non constant dimension used in context";
    }
  }
  else
  {
    auto t = apply_visitor(*this, e.e);

    //the type of E must be a constant, se we look for its unique bound
    //it is positive, so we want the lower constructed bound to be a constant

    auto& A = std::get<0>(t);
    auto& C = std::get<2>(t);
    TypeVariable var = get<TypeVariable>(std::get<1>(t));
    auto lower = C.lower(var);
    const Constant* lowerConstant = get<Constant>(&lower);

    if (C.predecessor(var).size() == 0 && lowerConstant != nullptr)
    {
      auto alpha = fresh();
      auto beta = fresh();
      auto lower = fresh();

      A.addConstantDim(*lowerConstant, lower, beta);

      C.add_to_closure(Constraint{beta, alpha});
      
      return std::make_tuple(A, alpha, C);
    }
    else
    {
      throw "Non constant dimension used in context";
    }

#if 0
    auto alpha = fresh();
    auto beta = fresh();
    auto gamma = fresh();

    ConstraintGraph& C = std::get<2>(t);
    TypeContext& A = std::get<0>(t);

    //C.add_to_closure(Constraint{alpha, beta});
    C.add_to_closure(Constraint{beta, alpha});
    C.add_to_closure(Constraint{std::get<1>(t), gamma});
    A.addDimension(gamma, std::make_pair(alpha, beta));

    return std::make_tuple(A, alpha, C);
#endif
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
  TypeContext A;
  ConstraintGraph C;

  TypeTuple result;

  result.record = fresh();

  for (const auto& p : e.pairs)
  {
    auto t_1 = apply_visitor(*this, p.first);
    auto t_2 = apply_visitor(*this, p.second);

    A.join(std::get<0>(t_1));
    A.join(std::get<0>(t_2));

    C.make_union(std::get<2>(t_1));
    C.make_union(std::get<2>(t_2));

    //t_1.tau is positive, so if it has a unique lower bound
    //that is a constant we have a constant dimension
    auto dimType = get<TypeVariable>(std::get<1>(t_1));
    Type lower = C.lower(dimType);
    if (C.predecessor(dimType).size() == 0 && C.successor(dimType).size() == 0
        && variant_is_type<Constant>(lower))
    {
      auto d = m_system.getDimensionIndex(get<Constant>(lower));
      result.types[d] = std::get<1>(t_2);
    }
  }

  auto alpha = fresh();
  C.add_to_closure(Constraint{result, alpha});

  return std::make_tuple(A, alpha, C);
}

TypeInferrer::result_type
TypeInferrer::operator()(const Tree::AtExpr& e)
{
  //the right hand side must be a tuple
  const Tree::TupleExpr* rhs = get<Tree::TupleExpr>(&e.rhs);

  if (rhs == nullptr)
  {
    throw "Right hand side of @ not a tuple";
  }

  auto lhst = apply_visitor(*this, e.lhs);

  TypeContext& A = std::get<0>(lhst);
  ConstraintGraph& C = std::get<2>(lhst);

  for (const auto& p : rhs->pairs)
  {
    //each dimension will either be a constant dimension, or passed as a 
    //parameter
    //so it is either #.d or evaluates to a constant

    auto t_1 = apply_visitor(*this, p.second);

    A.join(std::get<0>(t_1));
    C.make_union(std::get<2>(t_1));

    const Tree::HashExpr* h = get<Tree::HashExpr>(&p.first);
    const Tree::DimensionExpr* dim = nullptr;

    if (h != nullptr && (dim = get<Tree::DimensionExpr>(&h->e)) != nullptr)
    {
      //the dimension is passed as a parameter
      auto upper = fresh();
      auto value = fresh();
      auto thedim = fresh();

      A.addParamDim(dim->dim, thedim, value, upper);
      C.add_to_closure(Constraint{std::get<1>(t_1), value});
    }
    else
    {
      //the dimension is a constant
      auto t_0 = apply_visitor(*this, p.first);

      A.join(std::get<0>(t_0));
      C.make_union(std::get<2>(t_0));

      auto& C_0 = std::get<2>(t_0);
      TypeVariable v_0 = get<TypeVariable>(std::get<1>(t_0));
      Type lowerType = C_0.lower(v_0);
      const Constant* lower = get<Constant>(&lowerType);

      if (C_0.predecessor(v_0).size() == 0 && lower != nullptr)
      {
        A.addConstantDim(*lower, std::get<1>(t_1), fresh());
      }
      else
      {
        throw "Invalid dimension in context change";
      }
    }
  }

  return std::make_tuple(A, std::get<1>(lhst), C);
}

TypeInferrer::result_type
TypeInferrer::operator()(const Tree::LambdaExpr& e)
{
  auto t_0 = apply_visitor(*this, e.rhs);

  auto& C = std::get<2>(t_0);
  auto& context = std::get<0>(t_0);

  auto alpha = fresh();

  auto dim = context.has_entry(e.argDim) ? context.lookup(e.argDim) : fresh();

  C.add_to_closure(Constraint{
    TypeCBV{dim, std::get<1>(t_0)}, 
    alpha});

  //std::cout << "at lambda expression: dimension " << e.argDim << " has type "
  //  << print_type(context.lookup(e.argDim), m_system) << std::endl;

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
    auto dim = A.has_entry(d) ? A.lookup(d) : fresh();
    lhs.push_back(dim);
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

  t_0 = simplify(t_0);

  auto& A = std::get<0>(t_0);
  auto& C = std::get<2>(t_0);

  int dimCount = 0;
  for (const auto& d : e.dims)
  {
    auto t = apply_visitor(*this, d.second);

    A.join(std::get<0>(t));
    C.make_union(std::get<2>(t));

    // pull d out of the type context, allocate a new variable alpha, and set
    // inferred type of d <= alpha <= required type of d
    auto alpha = fresh();

    auto whichDim = e.dimAllocation[dimCount];

    auto dimConstant = Types::Dimension::create(whichDim);
    auto dimUse = A.lookup(dimConstant);

    C.add_to_closure(Constraint{alpha, dimUse.second});
    C.add_to_closure(Constraint{dimUse.first, alpha});

    C.add_to_closure(Constraint{alpha, A.lookup(whichDim)});
    C.add_to_closure(Constraint{std::get<1>(t), alpha});

    A.remove(whichDim);
    A.remove(dimConstant);

    ++dimCount;
  }

  return std::make_tuple(A, std::get<1>(t_0), C);
}

TypeInferrer::result_type
TypeInferrer::operator()(const Tree::ConditionalBestfitExpr& e)
{
  TypeContext A;
  ConstraintGraph C;

  TypeContext nonGuarded;

  auto alpha = fresh();

  std::map<dimension_index, Type> regionTypes;
  std::map<dimension_index, size_t> dimCounter;

  for (const auto& d : e.declarations)
  {
    std::vector<std::pair<dimension_index, Type>> currentRegionType;
    auto t_2 = apply_visitor(*this, std::get<3>(d));

    if (variant_is_type<Tree::RegionExpr>(std::get<1>(d)))
    {
      //get the dimensions out of the region, and the type that this
      //expression accepts is the union of all the types guarded for
      //at the same time, the type in this guard for that dimension must
      //be less than the required type for the dimension in the body

      auto guardA = process_region_guard(get<Tree::RegionExpr>(std::get<1>(d)),
        currentRegionType);

      A.join(guardA);

      //put the current types into the region
      for (const auto& dt : currentRegionType)
      {
        //std::cout << "processing dimension " << dt.first << std::endl;

        ++dimCounter[dt.first];
        auto iter = regionTypes.find(dt.first);
        if (iter == regionTypes.end())
        {
          regionTypes[dt.first] = dt.second;
        }
        else
        {
          //make atomic union
          if (variant_is_type<TypeAtomicUnion>(iter->second))
          {
            //add to union
            get<TypeAtomicUnion>(iter->second).add(dt.second);
          }
          else
          {
            //make a new union
            TypeAtomicUnion u;
            u.add(iter->second);
            u.add(dt.second);
            iter->second = u;
          }
        }
      }

      auto t_0 = apply_visitor(*this, std::get<1>(d));
      A.join(std::get<0>(t_0));
      C.make_union(std::get<2>(t_0));

      C.add_to_closure(Constraint{std::get<1>(t_0), TypeRegion()});
    }

    if (!variant_is_type<Tree::nil>(std::get<2>(d)))
    {
      auto t_1 = apply_visitor(*this, std::get<2>(d));
      nonGuarded.join(std::get<0>(t_1));
      C.make_union(std::get<2>(t_1));

      C.add_to_closure(Constraint{std::get<1>(t_1), 
        makeAtomic(m_system, U"bool")
      });
    }

    auto& A2 = std::get<0>(t_2);

    //anything mentioned in the guard comes out of non guarded
    for (const auto& g : currentRegionType)
    {
      //C.add_to_closure(
      A2.remove(g.first);
    }

    nonGuarded.join(A2);
    C.make_union(std::get<2>(t_2));

    C.add_to_closure(Constraint{std::get<1>(t_2), alpha});
  }

  for (const auto& rt : regionTypes)
  {
    //std::cout << "region type: " << rt.first << " :: " <<
    //  print_type(rt.second, m_system) << std::endl;

    TypeContext Atemp;
    //only add this dimension if it was mentioned in every guard
    //if it wasn't, then there is a guard that will accept it even if none
    //of the others match, therefore the accepted input is top

    //if it is mentioned in every guard, then it is the union type from the
    //guards, otherwise, it is the glb of the types based on their usage
    auto tyvar = fresh();
    if (dimCounter[rt.first] == e.declarations.size())
    {
      //if every guard mentioned it then use the union type
      C.add_to_closure(Constraint{tyvar, rt.second});
      Atemp.add(rt.first, tyvar);
      A.join(Atemp);
    }
    else
    {
      //otherwise use the required types from nonGuarded
      if (nonGuarded.has_entry(rt.first))
      {
        A.add(rt.first, nonGuarded.lookup(rt.first));
      }
    }

    nonGuarded.remove(rt.first);
  }

  //put in anything left that wasn't even mentioned in the guards
  A.join(nonGuarded);

  //std::cout << "returned context: " << A.print_context(m_system) << std::endl;
  //std::cout << "return C:\n" << C.print(m_system) << std::endl;

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

      //std::cout << "S = {";
      //print_container(std::cout, vars);
      //std::cout << "} |-> (" << gamma << ", " << lambda << ")" << std::endl;

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

class CanoniseRewriter
{
  public:

  CanoniseRewriter(Canonise& canon, bool positive)
  : m_canon(canon)
  , m_pos(positive)
  {
  }

  TypeVariable
  rename_var(TypeVariable v) const
  {
    return get<TypeVariable>(rewrite_type(v));
  }

  Type
  rewrite_type(const Type& t) const
  {
    if (m_pos)
    {
      return apply_visitor(m_canon, t, TagPositive());
    }
    else
    {
      return apply_visitor(m_canon, t, TagNegative());
    }
  }

  private:
  Canonise& m_canon;
  bool m_pos;
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
  TYPE_ATOMIC_UNION,
  TYPE_GLB,
  TYPE_LUB,
  TYPE_INTENSION,
  TYPE_CBV,
  TYPE_BASE,
  TYPE_DIM
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
  operator()(TypeAtomicUnion const&)
  {
    return HeadOrder::TYPE_ATOMIC_UNION;
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

struct CompareHead
{
  typedef bool result_type;

  bool
  less(const Type& a, const Type& b) const
  {
    return apply_visitor_double(*this, a, b);
  }

  bool
  operator()(const Constant& a, const Constant& b) const
  {
    return a < b;
  }

  bool
  operator()(const TypeAtomic& a, const TypeAtomic& b) const
  {
    return a.index < b.index;
  }

  bool
  operator()(const TypeAtomicUnion& a, const TypeAtomicUnion& b) const
  {
    if (a.constants < b.constants)
    {
      return true;
    }
    else
    {
      if (a.constants == b.constants)
      {
        return a.atomics < b.atomics;
      }
      else
      {
        return false;
      }
    }
  }

  template <typename A, typename B>
  bool
  operator()(const A& a, const B& b) const
  {
    GetTypeOrder order;
    return order(a) < order(b);
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

    //then sort by head of lower bound
    CompareHead head;

    const auto& aLower = C.lower(a);
    const auto& bLower = C.lower(b);

    if (head.less(aLower, bLower))
    {
      return true;
    }
    else if (head.less(bLower, aLower))
    {
      return false;
    }

    //then by head of upper bound
    const auto& aUpper = C.upper(a);
    const auto& bUpper = C.upper(b);

    if (head.less(aUpper, bUpper))
    {
      return true;
    }
    else if (head.less(bUpper, aUpper))
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

    CompareHead head;

    //a < b and b < a should be false, making them equal
    //otherwise they are not equal
    if (head.less(C.lower(a), C.lower(b)) || head.less(C.lower(b), C.lower(a)))
    {
      return false;
    }

    if (head.less(C.upper(a), C.upper(b)) || head.less(C.upper(b), C.upper(a)))
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

//a mapping from type variables to their inverse for each particular function
typedef std::map<TypeVariable, std::map<size_t, std::set<TypeVariable>>> 
  InverseSet;

struct MinimiseTransitions
{
  typedef TypeVariable result_type;

  TypeVariable
  map(const Type& t, size_t f) const
  {
    return apply_visitor(*this, t, f);
  }

  template <typename T>
  TypeVariable
  operator()(const T&, size_t f) const
  {
    return non;
  }

  TypeVariable
  operator()(const TypeCBV& cbv, size_t f) const
  {
    switch (f)
    {
      case 0:
      return get<TypeVariable>(cbv.lhs);

      case 1:
      return get<TypeVariable>(cbv.rhs);

      default:
      return non;
    }
  }

  TypeVariable
  operator()(const TypeBase& base, size_t f) const
  {
    if (base.lhs.size() < f)
    {
      return non;
    }

    if (f == 0)
    {
      return get<TypeVariable>(base.rhs);
    }
    else
    {
      return get<TypeVariable>(base.lhs.at(f-1));
    }
  }

  TypeVariable
  operator()(const TypeIntension& inten, size_t f) const
  {
    if (f == 0)
    {
      return get<TypeVariable>(inten.body);
    }

    return 0;
  }

  private:
  TypeVariable non = 0;
};

struct MinimiseCountFunctions
{
  typedef size_t result_type;

  size_t
  count(const Type& t) const
  {
    return apply_visitor(*this, t);
  }

  template <typename T>
  size_t
  operator()(const T&) const
  {
    return 0;
  }

  size_t
  operator()(const TypeCBV& cbv) const
  {
    return 2;
  }

  size_t
  operator()(const TypeBase& base) const
  {
    return base.lhs.size() + 1;
  }

  size_t
  operator()(const TypeIntension& inten) const
  {
    return 1;
  }
};

TypeVariable
minimise_transition(const ConstraintGraph& C, TypeVariable v, int f)
{
  MinimiseTransitions transition;
  MinimiseCountFunctions counter;

  auto lower = C.lower(v);
  auto upper = C.upper(v);

  auto l = counter.count(lower);
  auto u = counter.count(upper);

  if (l > u)
  {
    return transition.map(lower, f);
  }
  else
  {
    return transition.map(upper, f);
  }
}

//returns the set and the number of functions
std::pair<InverseSet, size_t>
generateInverse(const ConstraintGraph& C)
{
  InverseSet inverse;
  MinimiseCountFunctions counter;
  MinimiseTransitions transition;

  size_t maxFuns = 0;

  auto vars = C.domain();

  for (auto v : vars)
  {
    auto lower = C.lower(v);
    auto upper = C.upper(v);

    auto l = counter.count(lower);
    auto u = counter.count(upper);

    //only l or u will have a meaningful type, the one with a count greater
    //than 0 will be it
    Type* toMap = nullptr;
    size_t numFuns = 0;
    if (l != 0 || u != 0)
    {
      if (l > u)
      {
        toMap = &lower;
        numFuns = l;
      }
      else
      {
        toMap = &upper;
        numFuns = u;
      }

      maxFuns = std::max(maxFuns, numFuns);

      for (size_t i = 0; i != numFuns; ++i)
      {
        auto r = transition.map(*toMap, i);
        (inverse[r][i]).insert(v);
      }
    }
  }

  return std::make_pair(inverse, maxFuns);
}

template <typename Set, typename T>
struct SetCaller
{
  SetCaller(Set& s, const T& t)
  : m_s(s), m_t(t)
  {
    s.insert(t);
  }

  ~SetCaller()
  {
    m_s.erase(m_t);
  }

  operator Set& ()
  {
    return m_s;
  }

  Set& m_s;
  T m_t;
};

struct ReplaceDisplay : private GenericTypeTransformer<ReplaceDisplay>
{
  typedef Type result_type;
  using GenericTypeTransformer::operator();

  typedef SetCaller<std::set<TypeVariable>, TypeVariable> MySetCaller;

  ReplaceDisplay(const TypeScheme& t)
  : m_p(polarity(t))
  , m_A(std::get<0>(t))
  , m_type(std::get<1>(t))
  , m_C(std::get<2>(t))
  {
  }

  Type
  rewrite_type(const Type& t)
  {
    return replace(t);
  }

  TypeScheme
  replace()
  {
    //replace the type, then the type context, then try to replace
    //what is left in the context
    auto t = replace(m_type);

    TypeContext A = TypeContext::rewrite(m_A, *this);
    ConstraintGraph C;

    std::map
    <
      TypeVariable, 
      std::pair<std::vector<TypeVariable>, std::vector<TypeVariable>>
    > inGraph;

    VarSet inGraphVars;

    while (!m_unreplaced.empty())
    {
      auto v = *m_unreplaced.begin();

      inGraphVars.insert(v);

      auto lower = replace(m_C.lower(v));
      auto upper = replace(m_C.upper(v));

      //we only need any successors and predecessors if they are mentioned
      //in the graph, but we can't know that unless we have visited everything
      C.setLower(v, lower);
      C.setUpper(v, upper);

      inGraph[v] = std::make_pair(m_C.predecessor(v), m_C.successor(v));

      m_unreplaced.erase(v);
    }

    //now we can go through the variables that still appear, and only set their
    //predecessors and successors if they also appear in the graph

    for (const auto& vv : inGraph)
    {
      std::vector<TypeVariable> remains;

      //use the intersection of the successor and predecessor with the 
      //variables actually used
      std::set_intersection(inGraphVars.begin(), inGraphVars.end(),
        vv.second.first.begin(), vv.second.first.end(),
        std::back_inserter(remains));

      C.setPredecessor(vv.first, remains);

      remains.clear();

      std::set_intersection(inGraphVars.begin(), inGraphVars.end(),
        vv.second.second.begin(), vv.second.second.end(),
        std::back_inserter(remains));

      C.setSuccessor(vv.first, remains);
    }

    return std::make_tuple(A, t, C);
  }

#if 0
  template <typename T>
  Type
  operator()(const T& t)
  {
    return t;
  }
#endif

  Type
  operator()(TypeVariable v, std::set<TypeVariable>& s)
  {
    Type bound;
    if (s.find(v) != s.end())
    {
      //we're already computing the unique bound for this variable
      //which also means that we now know that its unique bound is itself
      m_uniqueBounds[v] = v;
      m_unreplaced.insert(v);
      return v;
    }
    else
    {
      if (m_p.first.find(v) != m_p.first.end())
      {
        //negative uses upper bound
        auto upper = m_C.upper(v);
        auto succ = m_C.successor(v);
        if (succ.size() == 0)
        {
          bound = replace(upper, MySetCaller(s, v));
        }
        else if (succ.size() == 1 && (get<TypeTop>(&upper) != nullptr))
        {
          bound = succ[0];
          m_uniqueBounds[v] = succ[0];
        }
        else
        {
          //variable has no unique bound, in which case it is itself
          bound = v;
          m_uniqueBounds[v] = v;
          m_unreplaced.insert(v);
        }
      }
      else
      {
        //positive uses lower bound
        auto pred = m_C.predecessor(v);
        auto lower = m_C.lower(v);
        if (pred.size() == 0)
        {
          bound = replace(lower, MySetCaller(s, v));
        }
        else if (pred.size() == 1 && (get<TypeBot>(&lower) != nullptr))
        {
          //don't replace with the lower bound in this case,
          //we replace with the upper bound in the opposite case,
          //this way variables that are the same stay the same
          bound = v;
          m_uniqueBounds[v] = v;
        }
        else
        {
          bound = v;
          m_unreplaced.insert(v);
        }
      }
    }

    //now if we still don't know the unique bound of v, it is v, otherwise
    //it is the bound
    auto iter = m_uniqueBounds.find(v);
    if (iter != m_uniqueBounds.end())
    {
      return iter->second;
    }
    else
    {
      m_uniqueBounds[v] = bound;
      return bound;
    }
  }

  private:

  Type
  replace(const Type& t)
  {
    std::set<TypeVariable> s;
    return replace(t, s);
  }

  Type
  replace(const Type& t, std::set<TypeVariable>& s)
  {
    return apply_visitor(*this, t, s);
  }

  const std::pair<VarSet, VarSet> m_p;
  const TypeContext& m_A;
  Type m_type;
  const ConstraintGraph& m_C;

  VarSet m_unreplaced;

  std::map<TypeVariable, Type> m_uniqueBounds;
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
  auto context = TypeContext::canonise(std::get<0>(t), 
    CanoniseRewriter(canon, true),
    CanoniseRewriter(canon, false)
  );

  //  std::bind(visitor_applier(), std::ref(canon), std::placeholders::_1,
  //    TagNegative())
  //);

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

  //conditional constraints remain as they are, because no constraint can have
  //a glb or lub term in it

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

      p.insert(result.first.begin(), result.first.end());
      n.insert(result.second.begin(), result.second.end());
    };

  auto gathercond = [&] (const std::vector<CondConstraint>& ccs) -> void
    {
      for (const auto& cc : ccs)
      {
        pos.insert(cc.lhs);
        neg.insert(cc.rhs);
      }
    };

  //mark the variables used as inputs to functions (type context) as negative
  std::get<0>(t).for_each(
    std::bind(gatherneg, std::placeholders::_1, std::ref(pos), std::ref(neg)));

  std::get<0>(t).markTLContext(
    std::bind(gatherpos, std::placeholders::_1, std::ref(pos), std::ref(neg)),
    std::bind(gatherneg, std::placeholders::_1, std::ref(pos), std::ref(neg))
  );

  //mark dimensions in the TL context
  #if 0
  const auto& dims = std::get<0>(t).getDimensions();
  for (const auto& d : dims)
  {
    pos.insert(d.first);
    gatherpos(d.second.first, pos, neg);
    gatherneg(d.second.second, pos, neg);

    //std::for_each(d.second.vars.begin(), d.second.vars.end(), 
    //  [&] (TypeVariable v) -> void
    //  {
    //    neg.insert(v);
    //  }
    //);
  }
  #endif

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

    std::get<2>(t).for_each_condition_if(
      gathercond,
      VarInSet(currentNeg)
    );
  }

  return std::make_pair(neg, pos);
}

TypeScheme
garbage_collect(const TypeScheme& t)
{
  auto p = polarity(t);

  #if 0
  std::cout << "positive: ";
  print_container(std::cout, p.second);
  std::cout << std::endl;
  std::cout << "negative: ";
  print_container(std::cout, p.first);
  std::cout << std::endl;
  #endif

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
  auto numFunctions = inverseResult.second;

  //first sort the type variables to get the initial partition
  std::sort(vars.begin(), vars.end(), compare);

  //std::cout << "sorted vars:" << std::endl;
  //std::copy(vars.begin(), vars.end(), 
  //  std::ostream_iterator<TypeVariable>(std::cout, ", "));
  //std::cout << std::endl;

  //divide into the initial partition of acceptable blocks
  //a block is a block number and a set of variables in that block
  std::map<size_t, MinimiseBlock> blocks;
  std::map<TypeVariable, size_t> inverseBlocks;

  size_t bAlloc = 0;
  int maxDist = 0;
  size_t maxBlock = 0;
  auto iter = vars.begin();
  auto start = iter;

  while (start != vars.end())
  {
    //there are at least two variables to deal with, so ++iter is defined
    ++iter;

    if (iter == vars.end() || !compare.equal(*start, *iter))
    {
      auto dist = iter - start;

      //make the block
      blocks.insert(std::make_pair(bAlloc, MinimiseBlock{{start, iter}}));

      ++bAlloc;

      start = iter;

      //keep track of the biggest block
      if (dist > maxDist)
      {
        maxBlock = bAlloc;
        maxDist = dist;
      }
    }
  }

  //record which block each variable is in
  for (const auto& block : blocks)
  {
    for (auto v : block.second.vars)
    {
      inverseBlocks.insert(std::make_pair(v, block.first));
    }
  }

  //now we try to reduce the size of the blocks we already have

  //all the splits left to do: each block mapped to each function
  std::map<size_t, std::set<size_t>> toSplit;

  //also keep a list of iterators into the splits so that we can keep a queue
  //easily
  std::list<
    std::pair<decltype(toSplit)::iterator,
      decltype(toSplit)::mapped_type::iterator>>
  splitQueue;

  //initially partition by all functions
  std::set<size_t> funpart;
  for (size_t f = 0; f != numFunctions; ++f)
  {
    funpart.insert(f);
  }
  
  #if 0
  std::cout << "blocks: ";
  for (const auto& b : blocks)
  {
    std::cout << "(";
    print_container(std::cout, b.second.vars);
    std::cout << ") ";
  }
  std::cout << std::endl;
  #endif

  // == Initialise ==

  auto addToQueue = [&] (size_t b, size_t fun) -> void
  {
    //std::cout << "adding split (" << b << ", " << fun << ") (";
    //print_container(std::cout, blocks[b].vars);
    //std::cout << ")" << std::endl;

    auto r1 = toSplit.insert(std::make_pair(b, std::set<size_t>()));

    auto r2 = r1.first->second.insert(fun);

    splitQueue.push_back(std::make_pair(r1.first, r2.first));
  };

  //partition with respect to all but the biggest and every seen variable
  for (const auto& block : blocks)
  {
    if (block.first != maxBlock)
    {
      #if 0
      auto result = toSplit.insert(std::make_pair(block.first, funpart)).first;

      auto iter = result->second.begin();
      while (iter != result->second.end())
      {
        splitQueue.push_back(std::make_pair(result, iter));
        ++iter;
      }
      #endif

      auto iter = funpart.begin();
      while (iter != funpart.end())
      {
        addToQueue(block.first, *iter);
        ++iter;
      }
    }
  }

  #if 0
  std::cout << "Partition by:" << std::endl;
  for (auto s : splitQueue)
  {
    std::cout << s.first->first << ", " << *s.second << std::endl;
  }
  #endif

  // == while L != empty do ==
  while (!splitQueue.empty())
  {
    // == b: Pick one pair (B_j, a) \in L ==
    //pick something out of the list
    auto currentSplit = *splitQueue.begin();
    auto currentBlock = currentSplit.first->first;
    auto currentFun = *currentSplit.second;

    //remove the first one from the list
    splitQueue.pop_front();
    currentSplit.first->second.erase(currentSplit.second);

    //std::cout << "splitting by (" << currentBlock << ", " 
    //  << currentFun << ")" << std::endl;

    //get the inverse of the current block and symbol
    // == c: Determine splittings of all blocks wrt (B_j, a) ==
    std::set<TypeVariable> currentInverse;
    for (auto s : blocks.find(currentSplit.first->first)->second.vars)
    {
      auto inverseIter = inverse.find(s);

      //we might not have an inverse
      if (inverseIter != inverse.end())
      {
        auto inverseFunIter = inverseIter->second.find(currentFun);

        if (inverseFunIter != inverseIter->second.end())
        {
          currentInverse.insert(inverseFunIter->second.begin(), 
            inverseFunIter->second.end());
        }
      }
    }

    // == e: Split each block as determined in c ==

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
    std::map<size_t, std::set<TypeVariable>> twins;
    for (auto b : seenBlocks)
    {
      auto bi = blocks.find(b);

      //std::cout << "checking block " << b << std::endl;

      //for the current block B for each element s of B, all the mappings 
      //for each function f(s)
      std::vector<std::vector<TypeVariable>> mappings;
      for (auto s : bi->second.vars)
      {
        auto mapped = minimise_transition(C, s, currentFun);

        if (inverseBlocks[mapped] == currentBlock)
        {
          //nothing to do here
        }
        else
        {
          //move this var to its twin
          //std::cout << "moving var " << s << " to twin" << std::endl;
          twins[b].insert(s);
        }
      }
    }

    // == f: Fix L according to splits that occurred ==
    //make the twins and split
    for (const auto& bi : twins)
    {
      auto i = blocks.find(bi.first);
      std::for_each(bi.second.begin(), bi.second.end(),
        [&] (TypeVariable v) -> void
        {
          //remove the variable from the current block
          i->second.vars.erase(v);
          //move each variables inverse block to the new block
          inverseBlocks[v] = bAlloc;
        }
      );
      blocks[bAlloc].vars = bi.second;
      auto bk = bAlloc;
      ++bAlloc;

      for (size_t c = 0; c != numFunctions; ++c)
      {
        //if (bi,c) \in L then add (bk,c)
        //otherwise add the smaller
        auto biter = toSplit.find(bi.first);

        if (biter != toSplit.end() && 
            biter->second.find(c) != biter->second.end())
        {
          //toSplit[bk].insert(c);
          addToQueue(bk, c);
        }
        else
        {
          if (bi.second.size() > i->second.vars.size())
          {
            //toSplit[i->first].insert(c);
            addToQueue(i->first, c);
          }
          else
          {
            //toSplit[bk].insert(c);
            addToQueue(bk, c);
          }
        }
      }
    }
  }

  std::map<TypeVariable, TypeVariable> replace;

  //std::cout << "blocks: ";
  for (const auto& b : blocks)
  {
    //std::cout << "(";
    //print_container(std::cout, b.second.vars);
    //std::cout << ") ";

    if (b.second.vars.size() > 1)
    {
      //replace every occurence of variables in the block with the
      //first variable
      auto iter = b.second.vars.begin();
      auto first = *iter;

      ++iter;

      while (iter != b.second.vars.end())
      {
        replace.insert(std::make_pair(*iter, first));
        ++iter;
      }
    }
  }
  //std::cout << std::endl;

  ConstraintGraph C1 = C;

  for (const auto& r : replace)
  {
    //first remove all variables in dom replace
    C1.erase_var(r.first);
  }

  RenamePolicyPreserve policy{replace};
  auto renamer = make_renamer(policy);
  auto t2 = renamer.rename(TypeScheme(std::get<0>(t), std::get<1>(t), C1));

  return t2;
  //return t;
}

TypeScheme
display_type(const TypeScheme& t)
{
  //keep a map of the free variables in each variable's constructed bound
  ReplaceDisplay replace(t);

  auto display = replace.replace();

  return display;
}

std::tuple<u32string, u32string, u32string>
display_type_scheme(const TypeScheme& t, System& system)
{
  TypePrinter printer(system, true);
}

}
}
