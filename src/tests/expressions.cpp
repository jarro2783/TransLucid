#include <tl/translator.hpp>
#include <tl/builtin_types.hpp>
#include <algorithm>
#include <tl/utility.hpp>
#include <string>


#if 0
#include <boost/spirit/home/phoenix/statement/if.hpp>
#include <boost/spirit/home/phoenix/operator/comparison.hpp>
#include <boost/spirit/home/phoenix/operator/logical.hpp>
#include <boost/spirit/home/phoenix/operator/arithmetic.hpp>
#include <boost/spirit/home/phoenix/operator/self.hpp>
#endif

#define BOOST_TEST_MODULE expressions
#include <boost/test/included/unit_test.hpp>

namespace TL = TransLucid;

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

struct translator_class {
  TL::Translator translator;
};

BOOST_FIXTURE_TEST_SUITE( expressions_tests, translator_class )

//BOOST_AUTO_TEST_SUITE( expressions_tests )

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

BOOST_AUTO_TEST_CASE ( strings ) {

  TL::HD *h;

  h = translator.translate_expr(L"«hello é world»");

  TL::TaggedValue v = (*h)(TL::Tuple());
  std::u32string s = v.first.value<TL::String>().value();

  BOOST_CHECK(s == U"hello é world");
  //std::cout << TL::utf32_to_utf8(s) << std::endl;
  delete h;

}

BOOST_AUTO_TEST_CASE ( chars ) {

  TL::HD *h;

  h = translator.translate_expr(L"\'è\'");
  TL::TaggedValue v = (*h)(TL::Tuple());

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
        TL::HD *h = translator.translate_expr(s.first);
        TL::TaggedValue v = (*h)(TL::Tuple());
        BOOST_CHECK_EQUAL(v.first.value<TL::Special>().value(),
                          s.second);
        delete h;
      });

}

BOOST_AUTO_TEST_SUITE_END()
