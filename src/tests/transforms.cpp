/* Cache tests.
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
 * @file transforms.cpp
 * Transformation tests.
 */

#include <tl/output.hpp>
#include <tl/system.hpp>
#include <tl/tree_printer.hpp>
#include <tl/utility.hpp>

#define CATCH_CONFIG_MAIN
#include "catch.hpp"

namespace TL = TransLucid;

TL::Tree::Expr
parseExpression(TL::System& s, const TL::u32string& input)
{
  TL::Parser::U32Iterator lineBegin(
    TL::Parser::makeUTF32Iterator(input.begin()));
  TL::Parser::U32Iterator lineEnd(
    TL::Parser::makeUTF32Iterator(input.end())
  );

  TL::Parser::StreamPosIterator posbegin(lineBegin, input,
    0,0);
  TL::Parser::StreamPosIterator posend(lineEnd);

  TL::Tree::Expr result;
  REQUIRE(s.parseExpression(posbegin, posend, result));

  return result;
}
