#define BOOST_TEST_MODULE expressions
#include <boost/test/included/unit_test.hpp>

#include <tl/tree_printer.hpp>
#include <tl/ast.hpp>

using namespace TransLucid;

BOOST_AUTO_TEST_CASE ( integer )
{
  Tree::Expr ast = mpz_class(42);

  typedef std::back_insert_iterator<std::string> out_iter;

  Printer::ExprPrinter<out_iter> print_grammar;
  std::string generated;
  std::back_insert_iterator<std::string> outit(generated);

  BOOST_CHECK(Printer::karma::generate(outit, print_grammar, ast));

  BOOST_CHECK_EQUAL(generated, "42");
}
