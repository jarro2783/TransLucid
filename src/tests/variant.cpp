#include <tl/variant.hpp>

#define CATCH_CONFIG_MAIN
#include "catch.hpp"

struct A
{
  int x;
};

struct B
{
  int y;
};

struct Visitor
{
  typedef int result_type;

  int
  operator()(const A& a)
  {
    return a.x;
  }

  int operator()(const B& b)
  {
    return b.y;
  }

  int operator()(int i)
  {
    return i;
  }
};

TEST_CASE ( "basic variant", "does the variant basic functionality work" )
{
  typedef TransLucid::Variant <int, A, B> var;
  var a(A{4});

  CHECK(a.apply_visitor(Visitor()) == 4);

  var b = a;
}
