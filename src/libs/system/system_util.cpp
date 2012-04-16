/* System utility functions
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

#include <tl/system.hpp>

namespace TransLucid
{

void
addPrinter(System& s, const u32string& type, const u32string& basefn)
{
  //add two functions, print and print_typename
  //print.c [c : type] = basefn!c;;
  //print_typename [c : type] = "type";;

  s.addFunction
  (
    Parser::FnDecl
    {
      U"print",
      {{Parser::FnDecl::ArgType::CALL_BY_VALUE, U"c"}},
      Tree::TupleExpr({{Tree::IdentExpr(U"c"), type}}),
      Tree::Expr(),
      Tree::BangAppExpr(Tree::IdentExpr(basefn), Tree::IdentExpr(U"c"))
    }
  );

  s.addFunction
  (
    Parser::FnDecl
    {
      U"print_typename",
      {{Parser::FnDecl::ArgType::CALL_BY_VALUE, U"c"}},
      Tree::TupleExpr({{Tree::IdentExpr(U"c"), type}}),
      Tree::Expr(),
      type
    }
  );

}

void
addConstructor(System& s, const u32string& type, const u32string& basefn)
{
}

void
addTypeEquation(System& s, const u32string& type)
{
  s.addEquation(Parser::Equation{U"type", Tree::Expr(), Tree::Expr(),
    Tree::LiteralExpr(U"type", type)});
}

}
