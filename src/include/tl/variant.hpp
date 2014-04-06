/* Juice Variant wrapper
   Copyright (C) 2014 Jarryd Beck

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
 * @file ast.hpp
 * Everything related to abstract syntax trees.
 * Contains everything needed to build and traverse an abstract syntax tree.
 */

#ifndef TL_VARIANT_HPP_INCLUDED
#define TL_VARIANT_HPP_INCLUDED

#include <tl/juice/variant.hpp>

namespace TransLucid
{
  using Juice::Variant;
  using Juice::recursive_wrapper;
  using Juice::get;
  using Juice::visitor_applier;
  using Juice::apply_visitor;
  using Juice::apply_visitor_binary;
  using Juice::variant_is_type;
}

#endif
