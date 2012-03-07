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

namespace TransLucid
{
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

    freeValues.push_back(std::make_pair(
      v.second, value
    ));
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

}
