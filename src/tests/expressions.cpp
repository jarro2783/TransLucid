/* Expression tests.
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
#include <tl/builtin_types.hpp>
#include <algorithm>
#include <tl/utility.hpp>
#include <string>
#include <tl/tree_printer.hpp>
#include <tl/header_parser.hpp>

#define BOOST_TEST_MODULE expressions
#include <boost/test/included/unit_test.hpp>

namespace TL = TransLucid;

typedef std::back_insert_iterator<std::string> print_iter;
TL::Printer::ExprPrinter<print_iter> print_grammar;


namespace
{
  TL::Translator translator;
}

struct translator_class {
  translator_class()
  {
    TL::Parser::Header& header = translator.header();
    TL::Parser::addBinaryOpSymbol
    (
      header, L"+", L"operator+", TL::Tree::ASSOC_LEFT, 5
    );
    TL::Parser::addBinaryOpSymbol
    (
      header, L"*", L"operator*", TL::Tree::ASSOC_LEFT, 10
    );
    TL::Parser::addBinaryOpSymbol
    (
      header, L"-", L"operator-", TL::Tree::ASSOC_LEFT, 5
    );
    TL::Parser::addUnaryOpSymbol
    (
      header, TL::Tree::UNARY_PREFIX, L"-", L"operator-"
    );
    TL::Parser::addUnaryOpSymbol
    (
      header, TL::Tree::UNARY_POSTFIX, L"-", L"operator-"
    );
    translator.loadLibrary(U"int");
  }
};

BOOST_GLOBAL_FIXTURE ( translator_class );

void
test_base(size_t base, uint32_t number, TL::Translator& t)
{
  std::string value = "0";
  mpz_class baseVal(base);
  value += baseVal.get_str(62);
  mpz_class num(number);
  if (base <= 36) {
    value += num.get_str(-base);
  } else {
    value += num.get_str(base);
  }

  TL::HD* h = t.translate_expr(TL::Parser::string_type(value.begin(),
                                                       value.end()));
  TL::TaggedValue v = (*h)(TL::Tuple());

  uint32_t result = v.first.value<TL::Intmp>().value().get_ui();
  BOOST_CHECK_EQUAL(result, (unsigned)number);

  delete h;
}

void
test_integer(int n, TL::Translator& translator)
{
  TL::HD* h = translator.translate_expr(std::to_wstring(n));
  TL::TaggedValue v = (*h)(TL::Tuple());
  //std::cout << v.first.value<TL::Intmp>().value().get_ui() << std::endl;
  BOOST_CHECK_EQUAL(v.first.value<TL::Intmp>().value().get_ui(), (unsigned)n);
  delete h;
}

BOOST_AUTO_TEST_SUITE( expressions_tests )

#ifndef DISABLE_INTEGER_TESTS

BOOST_AUTO_TEST_CASE( integers )
{
  for (int i = 0; i != 500; ++i)
  {
    test_integer(i, translator);
  }

  for (int i = 0; i != 1000; ++i)
  {
    for (int j = 2; j != 62; ++j)
    {
      test_base(j, i, translator);
    }
  }

}

#endif

BOOST_AUTO_TEST_CASE ( strings ) {

  TL::HD* h;

  h = translator.translate_expr(L"«hello é world»");

  TL::TaggedValue v = (*h)(TL::Tuple());
  std::u32string s = v.first.value<TL::String>().value();

  BOOST_CHECK(s == U"hello é world");
  //std::cout << TL::utf32_to_utf8(s) << std::endl;
  delete h;

}

BOOST_AUTO_TEST_CASE ( chars ) {

  TL::HD* h;

  h = translator.translate_expr(L"\'è\'");
  TL::TaggedValue v = (*h)(TL::Tuple());

  std::string generated;
  print_iter outit(generated);
  TL::Printer::karma::generate(outit, print_grammar,
                               translator.lastExpression());

  //std::cerr << "tree:" << std::endl
  //<< generated <<
  //"end tree" << std::endl;

  BOOST_REQUIRE_EQUAL(v.first.index(), TL::TYPE_INDEX_UCHAR);
  BOOST_CHECK(v.first.value<TL::Char>().value() == U'è');
  delete h;
}

BOOST_AUTO_TEST_CASE ( specials ) {

  using TL::Special;

  typedef std::list<std::pair<const wchar_t*, Special::Value>> SpecialList;
  SpecialList specials =
  {
    {L"sperror", Special::ERROR},
    {L"spaccess", Special::ACCESS},
    {L"sptype", Special::TYPEERROR},
    {L"spdim", Special::DIMENSION},
    {L"spundef", Special::UNDEF},
    {L"const", Special::CONST},
    {L"loop", Special::LOOP}
  };

  std::for_each(specials.begin(), specials.end(),
    [&translator] (SpecialList::value_type& s)
      {
        TL::HD* h = translator.translate_expr(s.first);
        TL::TaggedValue v = (*h)(TL::Tuple());
        BOOST_REQUIRE_EQUAL(v.first.index(), TL::TYPE_INDEX_SPECIAL);
        BOOST_CHECK_EQUAL(v.first.value<TL::Special>().value(),
                          s.second);
        delete h;
      });

}

BOOST_AUTO_TEST_CASE ( unary_ops )
{
}

BOOST_AUTO_TEST_CASE ( context_change )
{
  TL::HD* h = 0;

  h = translator.translate_expr(L"1");
  BOOST_CHECK_EQUAL
  (
    TL::get_dimension_index
    (
      &translator.system(),
      (*h)(TL::Tuple()).first
    ),
    TL::get_dimension_index
    (
      &translator.system(),
      (*h)(TL::Tuple()).first
    )
  );

  TL::HD* context1 = translator.translate_expr(L"[1 : 5]");
  TL::TaggedValue tuple1 = (*context1)(TL::Tuple());
  BOOST_REQUIRE_EQUAL(tuple1.first.index(), TL::TYPE_INDEX_TUPLE);

  h = translator.translate_expr(L"#1");
  TL::TaggedValue v = (*h)(tuple1.first.value<TL::Tuple>());
  BOOST_REQUIRE_EQUAL(v.first.index(), TL::TYPE_INDEX_INTMP);
  BOOST_CHECK_EQUAL(v.first.value<TL::Intmp>().value(), 5);

  TL::HD* context2 = translator.translate_expr(L"[1 : 42]");
  TL::TaggedValue tuple2 = (*context2)(TL::Tuple());
  BOOST_REQUIRE_EQUAL(tuple2.first.index(), TL::TYPE_INDEX_TUPLE);

  v = (*h)(tuple2.first.value<TL::Tuple>());
  BOOST_REQUIRE_EQUAL(v.first.index(), TL::TYPE_INDEX_INTMP);
  BOOST_CHECK_EQUAL(v.first.value<TL::Intmp>().value(), 42);

  TL::HD* context3 = translator.translate_expr(L"[1 : 42, 2 : 16, 3 : 47]");
  TL::TaggedValue tuple3 = (*context3)(TL::Tuple());
  BOOST_REQUIRE_EQUAL(tuple3.first.index(), TL::TYPE_INDEX_TUPLE);

  v = (*h)(tuple3.first.value<TL::Tuple>());
  BOOST_REQUIRE_EQUAL(v.first.index(), TL::TYPE_INDEX_INTMP);
  BOOST_CHECK_EQUAL(v.first.value<TL::Intmp>().value(), 42);

  h = translator.translate_expr(L"(#1-2) @ [1 : 5]");
  v = (*h)(TL::Tuple());

  BOOST_REQUIRE_EQUAL(v.first.index(), TL::TYPE_INDEX_INTMP);
  BOOST_CHECK_EQUAL(v.first.value<TL::Intmp>().value(), 3);
}

BOOST_AUTO_TEST_CASE ( header )
{
  TL::Translator t2;
  t2.parse_header
  (
    U"delimiter uchar<\"> uchar<\"> ustring<ustring>;;"
    U"prefix ustring<-> ustring<operator->;;"
    U"infixl ustring<%> ustring<operator%> 20;;"
  );

  TL::HD *h = t2.translate_expr(L"4 % 5");

  BOOST_REQUIRE(h != 0);
}

BOOST_AUTO_TEST_SUITE_END()
