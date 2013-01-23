/* Represents various semantic objects.
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

/**
 * @file semantics.hpp
 * Classes for representing semantics.
 */

#ifndef TL_SEMANTICS_HPP_INCLUDED
#define TL_SEMANTICS_HPP_INCLUDED

#include <tl/types.hpp>

#include <set>
#include <unordered_map>

namespace TransLucid
{

  /** 
   * The scope of an expression.
   * Represents the scope that an expression that will be a subexpression
   * of something else can have. This is for adding equations that are 
   * already inside another scope.
   *
   * An expression has the following scope:
   * - Dimensions to hold
   * - Identifiers to rename
   * - identifiers to convert to #!d
   */
  struct Scope
  {
    std::set<u32string> cbnParams;
    std::vector<dimension_index> scopeDims; 
    std::unordered_map<u32string, dimension_index> lookups;
    std::unordered_map<u32string, u32string> renames;
    std::set<u32string> dimscope;
  };

  typedef std::shared_ptr<Scope> ScopePtr;
}

#endif
