#include <boost/spirit/include/karma.hpp>
#include <tl/translator.hpp>
#include <tl/builtin_types.hpp>
#include <algorithm>
#include <tl/utility.hpp>
#include <string>
#include <boost/spirit/home/phoenix/statement/if.hpp>
#include <boost/spirit/home/phoenix/operator/comparison.hpp>
#include <boost/spirit/home/phoenix/operator/logical.hpp>
#include <boost/spirit/home/phoenix/operator/arithmetic.hpp>
#include <boost/spirit/home/phoenix/operator/self.hpp>

#define BOOST_TEST_MODULE expressions
#include <boost/test/included/unit_test.hpp>

namespace TL = TransLucid;

namespace karma = boost::spirit::karma;
namespace phoenix = boost::phoenix;

template <typename Iterator>
struct hex_number : public karma::grammar<Iterator, std::vector<int>()>
{
  hex_number()
  : hex_number<Iterator>::base_type(start)
  {
    using namespace karma::labels;
    namespace sp = boost::spirit;
    using karma::char_;
    using phoenix::if_;

    start =
        "0"
    <<  base
    <<  karma::int_
    ;

    base =
      char_("a-zA-Z2-9")
      [
        if_(_val >= 2 && _val <= 9)
        [
          sp::_1 = _val + '0'
        ]
        .else_
        [
          if_(_val >= 10 && _val <= 36)
          [
            sp::_1 = _val - 10 + 'a'
          ]
          .else_
          [
            sp::_1 = _val - 37 + 'A'
          ]
        ]
      ]
    ;
  }

  karma::rule<Iterator, std::vector<int>()> start;
  karma::rule<Iterator, int()> base;
};

void
test_base(int base, int number, TL::Translator& t)
{
  std::cout << "base = " << base << std::endl;
  std::string value = "0";
  mpz_class baseVal(base);
  value += baseVal.get_str(62);
  mpz_class num(number);
  value += num.get_str(base);

  TL::HD* h = t.translate_expr(TL::Parser::string_type(value.begin(),
                                                       value.end()));
  TL::TaggedValue v = (*h)(TL::Tuple());

  BOOST_CHECK_EQUAL(v.first.value<TL::Intmp>().value().get_ui(),
                    (unsigned)number);

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
  //TL::HD* h = translator.translate_expr(L"42");

  //TL::TaggedValue v = (*h)(TL::Tuple());
  //std::cout << v.first.value<TL::Intmp>().value().get_ui() << std::endl;
  //BOOST_CHECK_EQUAL(v.first.value<TL::Intmp>().value().get_ui(), 42);

  //delete h;

  for (int i = 0; i != 500; ++i)
  {
    test_integer(i, translator);
  }
  //test_integers<500> int_test(translator);
  //int_test.test();

  //test_bases<100, 62> base_test(translator);

  for (int i = 0; i != 1000; ++i)
  {
    for (int j = 2; j != 62; ++j)
    {
      test_base(j, i, translator);
    }
  }

#if 0
  std::string generated;
  std::back_insert_iterator<std::string> sink(generated);
  boost::spirit::karma::generate(
    sink,
    hex_number<decltype(sink)>(),
    std::vector<int>({62, 255})
    );
  std::cout << generated << std::endl;
#endif

  TL::HD* h = translator.translate_expr(L"0X10");
  TL::TaggedValue v = (*h)(TL::Tuple());
  BOOST_CHECK_EQUAL(v.first.value<TL::Intmp>().value().get_ui(), (unsigned)60);

  delete h;

  h = translator.translate_expr(L"«hello é world»");

  v = (*h)(TL::Tuple());
  std::u32string s = v.first.value<TL::String>().value();

  BOOST_CHECK(s == U"hello é world");
  //std::cout << TL::utf32_to_utf8(s) << std::endl;
  delete h;

  h = translator.translate_expr(L"\'è\'");
  v = (*h)(TL::Tuple());
  //std::cout << "char value = " << v.first.value<TL::Char>().value() << std::endl;
  //s.clear();
  //s += v.first.value<TL::Char>().value();
  //std::cout << TL::utf32_to_utf8(s) << std::endl;
  BOOST_CHECK(v.first.value<TL::Char>().value() == U'è');

  delete h;
  h = translator.translate_expr(L"spdim");
  v = (*h)(TL::Tuple());
  //std::cout << v.first.value<TL::Special>().value() << std::endl;
  BOOST_CHECK_EQUAL(v.first.value<TL::Special>().value(),
                    TL::Special::DIMENSION);

}

BOOST_AUTO_TEST_SUITE_END()
