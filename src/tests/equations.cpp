#include <tl/translator.hpp>
#include <tl/utility.hpp>
#include <tl/parser.hpp>

#define BOOST_TEST_MODULE equations
#include <boost/test/included/unit_test.hpp>

namespace TL = TransLucid;

struct translator_class {
  translator_class()
  {
    TL::Parser::Header& header = translator.header();
    TL::Parser::addOpSymbol(header, L"+", L"operator+", TL::Tree::ASSOC_LEFT, 5);
    TL::Parser::addOpSymbol(header, L"*", L"operator*", TL::Tree::ASSOC_LEFT, 10);
    TL::Parser::addOpSymbol(header, L"-", L"operator-", TL::Tree::ASSOC_LEFT, 5);
  }
  TL::Translator translator;
};

BOOST_FIXTURE_TEST_SUITE( expressions_tests, translator_class )

BOOST_AUTO_TEST_CASE ( single )
{
  translator.translate_and_add_equation_set(U" x = 5;; y = 6;;");

  TL::HD& system = translator.system();

  TL::TaggedValue v = system
  (TL::Tuple(TL::tuple_t(
    {
      {TL::DIM_ID, TL::generate_string(U"x")}
    }
  )));

  BOOST_REQUIRE_EQUAL(v.first.index(), TL::TYPE_INDEX_INTMP);
  BOOST_CHECK_EQUAL(v.first.value<TL::Intmp>().value(), 5);

  v = system
  (TL::Tuple(TL::tuple_t(
    {
      {TL::DIM_ID, TL::generate_string(U"y")}
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
  translator.loadLibrary(U"int");
  //TL::Parser::Header& header = translator.header();

  translator.translate_and_add_equation_set
  (
    U"a = 1 + 2;;"
    U"b = 5 * 6;;"
    U"c = 4 - 3;;"
  );

  TL::HD& system = translator.system();

  TL::TaggedValue v = system
  (TL::Tuple(TL::tuple_t(
    {
      {TL::DIM_ID, TL::generate_string(U"a")}
    }
  )));

  BOOST_REQUIRE_EQUAL(v.first.index(), TL::TYPE_INDEX_INTMP);
  BOOST_CHECK_EQUAL(v.first.value<TL::Intmp>().value(), 3);

  v = system
  (TL::Tuple(TL::tuple_t(
    {
      {TL::DIM_ID, TL::generate_string(U"b")}
    }
  )));

  BOOST_REQUIRE_EQUAL(v.first.index(), TL::TYPE_INDEX_INTMP);
  BOOST_CHECK_EQUAL(v.first.value<TL::Intmp>().value(), 30);

  v = system
  (TL::Tuple(TL::tuple_t(
    {
      {TL::DIM_ID, TL::generate_string(U"c")}
    }
  )));

  BOOST_REQUIRE_EQUAL(v.first.index(), TL::TYPE_INDEX_INTMP);
  BOOST_CHECK_EQUAL(v.first.value<TL::Intmp>().value(), 1);
}

BOOST_AUTO_TEST_CASE ( functions )
{
  translator.translate_and_add_equation_set
  (
    U"fib = fib @ [1 : #1-1] + fib @ [1 : #1-2];;"
    U"fib | [1 : 0] = 0;;"
    U"fib | [1 : 1] = 1;;"
  );

  TL::HD *h = translator.translate_expr(L"fib @ [1 : 2]");

  TL::TaggedValue v = (*h)(TL::Tuple());
  BOOST_REQUIRE_EQUAL(v.first.index(), TL::TYPE_INDEX_INTMP);
  BOOST_CHECK_EQUAL(v.first.value<TL::Intmp>().value(), 1);
}

BOOST_AUTO_TEST_SUITE_END()
