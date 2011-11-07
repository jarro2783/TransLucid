/* Parser tests.
   Copyright (C) 2009, 2010, 2011 Jarryd Beck and John Plaice

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

#include "tl/parser-new.hpp"
#include <tl/tree_printer.hpp>

#define CATCH_CONFIG_MAIN
#include "catch.hpp"

namespace TL = TransLucid;

TEST_CASE( "expr parser", "basic expression parser tests" )
{
  TL::System s;

  TL::Parser::Parser p(s);

  std::string input("42");

  TL::Parser::LexerIterator begin
  (
    TL::Parser::StreamPosIterator(
      TL::Parser::U32Iterator(
        TL::Parser::makeUTF8Iterator(input.begin()),
        TL::Parser::makeUTF8Iterator(input.end())
      ),
      "42"
    ),
    TL::Parser::StreamPosIterator(TL::Parser::U32Iterator()),
    s.getDefaultContext(),
    s.lookupIdentifiers()
  );

  TL::Parser::LexerIterator end = begin.makeEnd();
}
