/* Equation tests.
   Copyright (C) 2009, 2010 Jarryd Beck and John Plaice

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

#include <tl/translator.hpp>
#include <tl/utility.hpp>
//#include <tl/parser_util.hpp>
#include <tl/parser_fwd.hpp>
#include <tl/parser_header_util.hpp>

#define BOOST_TEST_MODULE equations
#include <boost/test/included/unit_test.hpp>

namespace TL = TransLucid;

namespace
{
  TL::Translator translator;
}

struct translator_class {
  translator_class()
  {
    translator.loadLibrary(U"int");
    TL::Parser::Header& header = translator.header();
    TL::Parser::addBinaryOpSymbol
    (
      header, U"+", U"operator+", TL::Tree::ASSOC_LEFT, 5
    );
    TL::Parser::addBinaryOpSymbol
    (
      header, U"*", U"operator*", TL::Tree::ASSOC_LEFT, 10
    );
    TL::Parser::addBinaryOpSymbol
    (
      header, U"-", U"operator-", TL::Tree::ASSOC_LEFT, 5
    );
    TL::Parser::addDimensionSymbol(header, U"n");
    TL::Parser::addDimensionSymbol(header, U"h");
    TL::Parser::addDimensionSymbol(header, U"i");
    TL::Parser::addDimensionSymbol(header, U"j");
  }
};

BOOST_GLOBAL_FIXTURE ( translator_class );

BOOST_AUTO_TEST_SUITE( expressions_tests )

BOOST_AUTO_TEST_CASE ( single )
{
  std::cerr << "First test case" << std::endl;
  translator.translate_and_add_equation_set(U" x = 5;; y = 6;;");

  TL::HD& system = translator.system();

  TL::TaggedConstant v = system
  (TL::Tuple(TL::tuple_t(
    {
      {TL::DIM_ID, TL::generate_string(U"x")},
      {TL::DIM_TIME, TL::makeTime(1)}
    }
  )));

  BOOST_REQUIRE_EQUAL(v.first.index(), TL::TYPE_INDEX_INTMP);
  BOOST_CHECK_EQUAL(v.first.value<TL::Intmp>().value(), 5);

  v = system
  (TL::Tuple(TL::tuple_t(
    {
      {TL::DIM_ID, TL::generate_string(U"y")},
      {TL::DIM_TIME, TL::makeTime(1)}
    }
  )));

  BOOST_REQUIRE_EQUAL(v.first.index(), TL::TYPE_INDEX_INTMP);
  BOOST_CHECK_EQUAL(v.first.value<TL::Intmp>().value(), 6);

  v = system
  (TL::Tuple(TL::tuple_t(
    {
      {TL::DIM_ID, TL::generate_string(U"z")}
    }
  )));

  BOOST_REQUIRE_EQUAL(v.first.index(), TL::TYPE_INDEX_SPECIAL);
  BOOST_CHECK_EQUAL(v.first.value<TL::Special>().value(), TL::Special::UNDEF);

  std::ostringstream os;
  os << v.first;

  BOOST_CHECK_EQUAL(os.str(), "special<undef>");
}

BOOST_AUTO_TEST_CASE ( simple_expressions )
{
  BOOST_TEST_MESSAGE("entering simple_expressions");
  //TL::Parser::Header& header = translator.header();

  translator.translate_and_add_equation_set
  (
    U"a = 1 + 2;;"
    U"b = 5 * 6;;"
    U"c = 4 - 3;;"
  );

  TL::HD& system = translator.system();

  TL::TaggedConstant v = system
  (TL::Tuple(TL::tuple_t(
    {
      {TL::DIM_ID, TL::generate_string(U"a")},
      {TL::DIM_TIME, TL::makeTime(1)}
    }
  )));

  BOOST_REQUIRE_EQUAL(v.first.index(), TL::TYPE_INDEX_INTMP);
  BOOST_CHECK_EQUAL(v.first.value<TL::Intmp>().value(), 3);

  v = system
  (TL::Tuple(TL::tuple_t(
    {
      {TL::DIM_ID, TL::generate_string(U"b")},
      {TL::DIM_TIME, TL::makeTime(1)}
    }
  )));

  BOOST_REQUIRE_EQUAL(v.first.index(), TL::TYPE_INDEX_INTMP);
  BOOST_CHECK_EQUAL(v.first.value<TL::Intmp>().value(), 30);

  v = system
  (TL::Tuple(TL::tuple_t(
    {
      {TL::DIM_ID, TL::generate_string(U"c")},
      {TL::DIM_TIME, TL::makeTime(1)}
    }
  )));

  BOOST_REQUIRE_EQUAL(v.first.index(), TL::TYPE_INDEX_INTMP);
  BOOST_CHECK_EQUAL(v.first.value<TL::Intmp>().value(), 1);

  translator.translate_and_add_equation_set
  (
    U"d = 1 + 2 * 3;;"
  );
  v = system
  (TL::Tuple(TL::tuple_t(
    {
      {TL::DIM_ID, TL::generate_string(U"d")},
      {TL::DIM_TIME, TL::makeTime(1)}
    }
  )));

  BOOST_REQUIRE_EQUAL(v.first.index(), TL::TYPE_INDEX_INTMP);
  BOOST_CHECK_EQUAL(v.first.value<TL::Intmp>().value(), 7);
}

BOOST_AUTO_TEST_CASE ( functions )
{
  std::cerr << "Third test case" << std::endl;
  TL::HD* h = 0;
  TL::TaggedConstant v;
  TL::Tuple k(TL::tuple_t({{TL::DIM_TIME, TL::makeTime(0)}}));

  translator.translate_and_add_equation_set
  (
    U"fib = (fib @ [1 : #1-1]) + (fib @ [1 : #1-2]);;"
    U"fib | [1 : 0] = 0;;"
    U"fib | [1 : 1] = 1;;"
  );

  h = translator.translate_expr(U"fib @ [1 : 0]");
  BOOST_REQUIRE(h != 0);
  v = (*h)(k);
  BOOST_REQUIRE_EQUAL(v.first.index(), TL::TYPE_INDEX_INTMP);
  BOOST_CHECK_EQUAL(v.first.value<TL::Intmp>().value(), 0);

  h = translator.translate_expr(U"fib @ [1 : 1]");
  BOOST_REQUIRE(h != 0);
  v = (*h)(k);
  BOOST_REQUIRE_EQUAL(v.first.index(), TL::TYPE_INDEX_INTMP);
  BOOST_CHECK_EQUAL(v.first.value<TL::Intmp>().value(), 1);

  h = translator.translate_expr(U"fib @ [1 : 2]");
  v = (*h)(k);
  BOOST_REQUIRE_EQUAL(v.first.index(), TL::TYPE_INDEX_INTMP);
  BOOST_CHECK_EQUAL(v.first.value<TL::Intmp>().value(), 1);

  #if 0
  BOOST_REQUIRE(
  translator.parse_header
  (
    U"dimension \"n\";;"
    U"dimension \"h\";;"
    U"dimension \"i\";;"
  )
  != false);
  #endif

  translator.translate_and_add_equation_set
  (
    U"fact = #n * (fact @ [n:#n-1]);;"
    U"fact | [n:0] = 1;;"
  );

  h = translator.translate_expr(U"(fact + #i) @ [time:0, n:3, i:25, h:50]");
  BOOST_REQUIRE(h != 0);
  v = (*h)(k);
  BOOST_REQUIRE_EQUAL(v.first.index(), TL::TYPE_INDEX_INTMP);
  BOOST_CHECK_EQUAL(v.first.value<TL::Intmp>().value(), 31);

  h = translator.translate_expr(
    U"(fact + #i) @ [time:0, n:3, i:25, h:50, j:60]");
  BOOST_REQUIRE(h != 0);
  v = (*h)(k);
  BOOST_REQUIRE_EQUAL(v.first.index(), TL::TYPE_INDEX_INTMP);
  BOOST_CHECK_EQUAL(v.first.value<TL::Intmp>().value(), 31);
}

BOOST_AUTO_TEST_SUITE_END()
