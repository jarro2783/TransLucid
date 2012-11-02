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

  s.addFunDeclParsed
  (
    Parser::FnDecl
    {
      U"print",
      //I think clang is broken, this should work without the next line
      std::vector<std::pair<Parser::FnDecl::ArgType, u32string>>
        {{Parser::FnDecl::ArgType::CALL_BY_VALUE, U"c"}},
      //gcc 4.7.0 appears to be broken
      //Tree::TupleExpr({{Tree::IdentExpr(U"c"), Tree::IdentExpr(type)}}),
      Tree::RegionExpr(Tree::RegionExpr::Entries
      {
        Tree::RegionExpr::Entry
        {
          Tree::IdentExpr(U"c"), 
          Region::Containment::IN,
          Tree::IdentExpr(type)
        }
      }),
      Tree::Expr(),
      Tree::BangAppExpr(Tree::IdentExpr(basefn), Tree::IdentExpr(U"c"))
    }
  );

  s.addFunDeclParsed
  (
    Parser::FnDecl
    {
      U"print_typename",
      //I think clang is broken, this should work without the next line
      std::vector<std::pair<Parser::FnDecl::ArgType, u32string>>
      {{Parser::FnDecl::ArgType::CALL_BY_VALUE, U"c"}},
      //same here
      //Tree::TupleExpr({{Tree::IdentExpr(U"c"), Tree::IdentExpr(type)}}),
      Tree::RegionExpr(Tree::RegionExpr::Entries
      {
        Tree::RegionExpr::Entry
        {
          Tree::IdentExpr(U"c"), 
          Region::Containment::IN,
          Tree::IdentExpr(type)
        }
      }),
      Tree::Expr(),
      type
    }
  );

}

void
addConstructor(System& s, const u32string& type, const u32string& basefn)
{
  s.addFunDeclParsed
  (
    Parser::FnDecl
    {
      U"construct_literal",
      //I think clang is broken, this should work without the next line
      std::vector<std::pair<Parser::FnDecl::ArgType, u32string>>
      {
        {Parser::FnDecl::ArgType::CALL_BY_VALUE, U"t"},
        {Parser::FnDecl::ArgType::CALL_BY_VALUE, U"v"}
      },
      Tree::RegionExpr({
        Tree::RegionExpr::Entry{
          Tree::IdentExpr(U"t"), 
          Region::Containment::IN,
          type
        }}),
      Tree::Expr(),
      Tree::BangAppExpr(Tree::IdentExpr(basefn), Tree::IdentExpr(U"v"))
    }
  );
}

void
addTypeEquation(System& s, const u32string& type)
{
  s.addVariableDeclParsed(Parser::Equation{type, Tree::Expr(), Tree::Expr(),
    Tree::LiteralExpr(U"typetype", type)});
}

}
