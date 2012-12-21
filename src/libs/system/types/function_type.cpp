/* Function type.
   Copyright (C) 2012 Jarryd Beck

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
#include <tl/types/demand.hpp>
#include <tl/types/function.hpp>
#include <tl/types/intension.hpp>
#include <tl/types/tuple.hpp>
#include <tl/types_util.hpp>
#include <tl/utility.hpp>

#include <tl/output.hpp>

namespace TransLucid
{

namespace
{

//work out function comparison
#if 0
bool function_less
(
  WS* lexpr,
  WS* rexpr,
  const std::vector<std::pair<dimension_index, Constant>>& lscope,
  const std::vector<std::pair<dimension_index, Constant>>& rscope
)
{
  //first compare the pointers
  if (lexpr < rexpr)
  {
    return true;
  }
  else if (!(rexpr < lexpr))
  {
    //they are the same function so compare
    //all the closure values
    auto liter = lscope.begin();
    auto riter = rscope.begin();

    while (liter != lscope.end())
    {
      if (liter->second < riter->second)
      {
        return true;
      }
      else if (riter->second < liter->second)
      {
        //the rhs is bigger
        return false;
      }

      ++liter;
      ++riter;
    }
  }
  else
  {
    //rhs is bigger so return false
    return false;
  }

  //if we got here then they are equal
  return false;
}
#endif

}

Constant
createValueFunction
(
  System *system,
  const u32string& name, 
  dimension_index argDim, 
  WS* expr,
  const std::vector<WS*>& binds,
  const std::vector<dimension_index>& scope,
  Context& kappa
)
{
  return Types::ValueFunction::create(
    ValueFunctionType(system, name, argDim, expr, binds, scope, kappa)
  );
}

//check for scope dimensions
//evaluate free variables, checking for demands
//then create the actual function
Constant
createValueFunctionCached
(
  System *system,
  const u32string& name, 
  dimension_index argDim, 
  WS* expr,
  Context& kappa,
  Context& delta
)
{
  //return Types::ValueFunction::create(
  //  ValueFunctionType(system, name, argDim, expr, kappa)
  //);
  #ifdef CACHE_TODO
  #warning implement cache here
  #endif
  return Constant();
}

//check for scope dimensions
//evaluate free variables, checking for demands
//then create the actual function
TimeConstant
createValueFunctionCachedNew
(
  System *system,
  const u32string& name, 
  dimension_index argDim, 
  WS* expr,
  const std::vector<WS*>& bindsws,
  const std::vector<dimension_index>& scope,
  Context& kappa,
  Delta& delta,
  const Thread& w, 
  size_t t
)
{
  //return Types::ValueFunction::create(
  //  ValueFunctionType(system, name, argDim, expr, kappa)
  //);
  std::vector<dimension_index> binds;
  std::vector<dimension_index> demands;
  size_t maxTime = 0;

  for (auto b : bindsws)
  {
    auto result = (*b)(kappa, delta, w, t);
    maxTime = std::max(maxTime, result.first);
    
    if (result.second.index() == TYPE_INDEX_DEMAND)
    {
      Types::Demand::append(result.second, demands);
    }
    else
    {
      binds.push_back(system->getDimensionIndex(result.second));
    }
  }

  if (!demands.empty())
  {
    return std::make_pair(maxTime, Types::Demand::create(demands));
  }

  for (auto d : binds)
  {
    if (delta.find(d) == delta.end())
    {
      demands.push_back(d);
    }
  }

  for (auto d : scope)
  {
    if (delta.find(d) == delta.end())
    {
      demands.push_back(d);
    }
  }

  if (!demands.empty())
  {
    return std::make_pair(maxTime, Types::Demand::create(demands));
  }

  return std::make_pair(maxTime, Types::ValueFunction::create(
    ValueFunctionType(system, name, argDim, expr, binds, scope, kappa)
  ));

  return TimeConstant();
}

Constant
ValueFunctionType::apply(Context& k, const Constant& value) const
{
  //set m_dim = value in the context and evaluate the expr
  ContextPerturber p{k, {{m_dim, value}}};

  //set the bound dimensions
  p.perturb(m_binds);

  return (*m_expr)(k);
}

Constant
ValueFunctionType::apply
(
  Context& kappa, 
  Context& delta, 
  const Constant& value
) const
{
  //set m_dim = value in the context and evaluate the expr
  ContextPerturber pkappa{kappa};
  ContextPerturber pdelta{delta};

  pkappa.perturb({{m_dim, value}});
  pdelta.perturb({{m_dim, value}});

  #ifdef CACHE_TODO
  #warning implement cache here
  #endif
  return Constant();
}

TimeConstant
ValueFunctionType::apply
(
  Context& kappa, 
  Delta& d, 
  const Thread& w, 
  size_t t,
  const Constant& value
) const
{
  //set m_dim = value in the context and evaluate the expr
  ContextPerturber p{kappa, {{m_dim, value}}};
  DeltaPerturber dp{d};

  dp.perturb(m_dim);
  dp.perturb(m_binds);

  //set the bound dimensions
  p.perturb(m_binds);

  return (*m_expr)(kappa, d, w, t);
}

bool
ValueFunctionType::less(const ValueFunctionType& rhs) const
{
  //this needs to be implemented properly
  return false;
  //return function_less(m_expr, rhs.m_expr, m_scopeDims, rhs.m_scopeDims);
}

namespace Types
{

namespace ValueFunction
{

bool
less(const Constant& lhs, const Constant& rhs)
{
  const auto& flhs = get_constant_pointer<ValueFunctionType>(lhs);
  const auto& frhs = get_constant_pointer<ValueFunctionType>(rhs);

  return flhs.less(frhs);
}

}

}

}
