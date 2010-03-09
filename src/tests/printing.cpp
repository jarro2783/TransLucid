#define BOOST_TEST_MODULE expressions
#include <boost/test/included/unit_test.hpp>

#include <tl/tree_printer.hpp>
#include <tl/translator.hpp>
#include <tl/ast.hpp>

using namespace TransLucid;

typedef std::back_insert_iterator<std::string> out_iter;

namespace
{

Translator translator;

Printer::ExprPrinter<out_iter> print_grammar;
std::string generated;
std::back_insert_iterator<std::string> outit(generated);

}

BOOST_AUTO_TEST_CASE ( integer )
{
  Tree::Expr ast = mpz_class(42);

  generated.clear();
  BOOST_CHECK(Printer::karma::generate(outit, print_grammar, ast));

  BOOST_CHECK_EQUAL(generated, "42");

  ast = Tree::BinaryOpExpr
  (
    Tree::BinaryOperation
    (
      Tree::ASSOC_LEFT,
      U"operator+",
      U"+",
      5
    ),
   mpz_class(10),
   mpz_class(15)
  );

  generated.clear();
  BOOST_CHECK(Printer::karma::generate(outit, print_grammar, ast));
  BOOST_CHECK_EQUAL(generated, "10+15");
}

BOOST_AUTO_TEST_CASE ( tuple )
{
  Parser::string_type input = L"[1:1]";
  translator.translate_expr(input);

  generated.clear();
  BOOST_CHECK(Printer::karma::generate
  (
    outit,
    print_grammar,
    translator.lastExpression()
  ));

  BOOST_CHECK_EQUAL(generated, "[1 : 1]");
}

BOOST_AUTO_TEST_CASE ( hash_expr )
{
  Parser::string_type input(L"#1");
  translator.translate_expr(input);

  generated.clear();
  BOOST_CHECK(Printer::karma::generate
  (
    outit,
    print_grammar,
    translator.lastExpression()
  ));

  BOOST_CHECK_EQUAL(generated, "#1");
}

BOOST_AUTO_TEST_CASE ( at )
{
  Parser::string_type input(L"#1 @ [1:2]");
  translator.translate_expr(input);

  generated.clear();
  BOOST_CHECK(Printer::karma::generate
  (
    outit,
    print_grammar,
    translator.lastExpression()
  ));

  BOOST_CHECK_EQUAL(generated, "#1@[1 : 2]");
}
