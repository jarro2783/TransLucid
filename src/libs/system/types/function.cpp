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
#include <tl/types/tuple.hpp>
#include <tl/types_util.hpp>
#include <tl/utility.hpp>

namespace TransLucid
{

namespace
{

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

}

NameFunctionType::NameFunctionType
(
  System* system,
  const u32string& name, 
  dimension_index argDim, 
  dimension_index odometerDim, 
  const std::vector<dimension_index>& scope,
  const std::vector<std::pair<dimension_index, Constant>>& free,
  WS* expr,
  Context& k
)
: m_name(name), m_argDim(argDim), m_odometerDim(odometerDim), m_expr(expr)
{
  for (auto d : scope)
  {
    m_scopeDims.push_back(std::make_pair(d, k.lookup(d)));
  }

  std::copy(free.begin(), free.end(), std::back_inserter(m_scopeDims));
}

Constant
createValueFunction
(
  System *system,
  const u32string& name, 
  dimension_index argDim, 
  const std::vector<dimension_index>& scope,
  const std::vector<std::pair<u32string, dimension_index>>& free,
  WS* expr,
  Context& kappa
)
{
  std::vector<std::pair<dimension_index, Constant>> freeValues;

  System::IdentifierLookup idents = system->lookupIdentifiers();

  //evaluate all of the free variables
  for (const auto& v : free)
  {
    auto var = idents.lookup(v.first);
    Constant value = var == nullptr ? Types::Special::create(SP_UNDEF)
      : (*idents.lookup(v.first))(kappa);

    freeValues.push_back(std::make_pair(
      v.second, value
    ));
  }

  return Types::ValueFunction::create(
    ValueFunctionType(system, name, argDim, scope, freeValues, expr, kappa)
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
  const std::vector<dimension_index>& scope,
  const std::vector<std::pair<u32string, dimension_index>>& free,
  WS* expr,
  Context& kappa,
  Context& delta
)
{
  std::vector<dimension_index> demands;
  std::vector<std::pair<dimension_index, Constant>> freeValues;

  System::IdentifierLookup idents = system->lookupIdentifiers();

  //evaluate all of the free variables
  for (const auto& v : free)
  {
    auto var = idents.lookup(v.first);
    Constant value = var == nullptr ? Types::Special::create(SP_UNDEF)
      : (*idents.lookup(v.first))(kappa, delta);

    if (value.index() == TYPE_INDEX_DEMAND)
    {
      Types::Demand::append(value, demands);
    }
    else
    {
      freeValues.push_back(std::make_pair(
        v.second, value
      ));
    }
  }

  if (demands.size() > 0)
  {
    return Types::Demand::create(demands);
  }

  return Types::ValueFunction::create(
    ValueFunctionType(system, name, argDim, scope, freeValues, expr, kappa)
  );
}

Constant
createNameFunction
(
  System *system,
  const u32string& name, 
  dimension_index argDim, 
  dimension_index odometerDim,
  const std::vector<dimension_index>& scope,
  const std::vector<std::pair<u32string, dimension_index>>& free,
  WS* expr,
  Context& kappa
)
{
  std::vector<std::pair<dimension_index, Constant>> freeValues;

  System::IdentifierLookup idents = system->lookupIdentifiers();

  //evaluate all of the free variables
  for (const auto& v : free)
  {
    auto var = idents.lookup(v.first);
    Constant value = var == nullptr ? Types::Special::create(SP_UNDEF)
      : (*idents.lookup(v.first))(kappa);

    freeValues.push_back(std::make_pair(
      v.second, value
    ));
  }

  return Types::NameFunction::create(
    NameFunctionType(system, name, argDim, odometerDim, 
      scope, freeValues, expr, kappa)
  );
}

//check for scope dimensions
//evaluate free variables, checking for demands
//then create the actual function
Constant
createNameFunctionCached
(
  System *system,
  const u32string& name, 
  dimension_index argDim, 
  dimension_index odometerDim,
  const std::vector<dimension_index>& scope,
  const std::vector<std::pair<u32string, dimension_index>>& free,
  WS* expr,
  Context& kappa,
  Context& delta
)
{
  std::vector<dimension_index> demands;
  std::vector<std::pair<dimension_index, Constant>> freeValues;

  System::IdentifierLookup idents = system->lookupIdentifiers();

  //evaluate all of the free variables
  for (const auto& v : free)
  {
    auto var = idents.lookup(v.first);
    Constant value = var == nullptr ? Types::Special::create(SP_UNDEF)
      : (*idents.lookup(v.first))(kappa, delta);

    if (value.index() == TYPE_INDEX_DEMAND)
    {
      Types::Demand::append(value, demands);
    }

    freeValues.push_back(std::make_pair(
      v.second, value
    ));
  }

  if (demands.size() > 0)
  {
    return Types::Demand::create(demands);
  }

  return Types::NameFunction::create(
    NameFunctionType
    (
      system,
      name,
      argDim,
      odometerDim,
      scope,
      freeValues,
      expr,
      kappa
    )
  );
}

Constant
ValueFunctionType::apply(Context& k, const Constant& value) const
{
  //set m_dim = value in the context and evaluate the expr
  ContextPerturber p(k, {{m_dim, value}});
  p.perturb(m_scopeDims);
  auto r = (*m_expr)(k);

  return r;
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
  ContextPerturber pkappa(kappa, {{m_dim, value}});
  ContextPerturber pdelta(delta, {{m_dim, value}});

  pkappa.perturb(m_scopeDims);
  pdelta.perturb(m_scopeDims);

  auto r = (*m_expr)(kappa, delta);

  return r;
}

Constant
NameFunctionType::apply
(
  Context& k, 
  const Constant& c,
  std::vector<dimension_index>& Lall
) const
{
  //add to the list of our args
  //pre: c is a workshop value

  //add to the list of odometers
  tuple_t odometer;
  for (auto d : Lall)
  {
    odometer.insert(std::make_pair(d, k.lookup(d)));
  }

  //argdim = cons(c, #argdim)
  //Tuple argList = makeList(c, k.lookup(m_argDim));
  //Tuple odometerList = makeList(Types::Tuple::create(Tuple(odometer)),
  //  k.lookup(m_odometerDim));

  //ContextPerturber p(k,
  //{
  //  {m_argDim, Types::Tuple::create(argList)},
  //  {m_odometerDim, Types::Tuple::create(odometerList)}
  //});

  ContextPerturber p(k,
  {
    {m_argDim, c},
    {m_odometerDim, Types::Tuple::create(Tuple(odometer))}
  });

  p.perturb(m_scopeDims);

  return (*m_expr)(k);
}

Constant
NameFunctionType::apply
(
  Context& kappa, 
  Context& delta, 
  const Constant& c,
  std::vector<dimension_index>& Lall
) const
{
  //add to the list of our args
  //pre: c is a workshop value

  //add to the list of odometers
  //std::vector<dimension_index> demands;
  tuple_t odometer;
  for (auto d : Lall)
  {
    #if 0
    if (!delta.has_entry(d))
    {
      demands.push_back(d);
    }
    #endif
    odometer.insert(std::make_pair(d, kappa.lookup(d)));
  }

  //first lookup the dimensions that we need
  //if (!delta.has_entry(m_argDim))
  //{
  //  demands.push_back(m_argDim);
  //}

  //if (!delta.has_entry(m_odometerDim))
  //{
  //  demands.push_back(m_odometerDim);
  //}

  #if 0
  if (!demands.empty())
  {
    return Types::Demand::create(demands);
  }
  #endif

  //argdim = cons(c, #argdim)
  //Tuple argList = makeList(c, kappa.lookup(m_argDim));
  //Tuple odometerList = makeList(Types::Tuple::create(Tuple(odometer)),
  //  kappa.lookup(m_odometerDim));

  std::initializer_list<std::pair<dimension_index, Constant>>
  toChange = {
    {m_argDim, c},
    {m_odometerDim, Types::Tuple::create(Tuple(odometer))}
  };

  ContextPerturber pkappa(kappa, toChange);
  ContextPerturber pdelta(delta, toChange);

  pkappa.perturb(m_scopeDims);
  pdelta.perturb(m_scopeDims);

  return (*m_expr)(kappa, delta);
}

bool
ValueFunctionType::less(const ValueFunctionType& rhs) const
{
  return function_less(m_expr, rhs.m_expr, m_scopeDims, rhs.m_scopeDims);
}

bool
NameFunctionType::less(const NameFunctionType& rhs) const
{
  return function_less(m_expr, rhs.m_expr, m_scopeDims, rhs.m_scopeDims);
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

namespace NameFunction
{

bool
less(const Constant& lhs, const Constant& rhs)
{
  const auto& flhs = get_constant_pointer<NameFunctionType>(lhs);
  const auto& frhs = get_constant_pointer<NameFunctionType>(rhs);

  return flhs.less(frhs);
}

}

}

}
