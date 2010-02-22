#include <tl/translator.hpp>

#define BOOST_TEST_MODULE equations
#include <boost/test/included/unit_test.hpp>

namespace TL = TransLucid;

struct translator_class {
  TL::Translator translator;
};

BOOST_FIXTURE_TEST_SUITE( expressions_tests, translator_class )

BOOST_AUTO_TEST_CASE ( single )
{
  translator.translate_equation_set(U"x = 5;; y = 6;;");
}

BOOST_AUTO_TEST_SUITE_END()
