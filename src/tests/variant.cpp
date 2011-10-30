#include <tl/variant.hpp>

#define CATCH_CONFIG_MAIN
#include "catch.hpp"

struct Sum
{
  int a;
  int b;
};

struct Multiply
{
  int a;
  int b;
};

struct Visitor
{
  typedef int result_type;

  int
  operator()(const Sum& s)
  {
    return s.a + s.b;
  }

  int operator()(const Multiply& m)
  {
    return m.a * m.b;
  }

  int operator()(int i)
  {
    return i;
  }
};

typedef TransLucid::Variant <int, Sum, Multiply> var;

TEST_CASE ( "basic variant", "does the variant basic functionality work" )
{
  var a(Sum{2,2});

  CHECK(a.apply_visitor(Visitor()) == 4);

  var b = a;
  CHECK(b.apply_visitor(Visitor()) == 4);

  var c;
  c = b;
  CHECK(c.apply_visitor(Visitor()) == 4);
}

TEST_CASE ("variant move semantics", 
  "variant with all the move operators")
{
  var a{var{Sum{5,10}}};  

  CHECK(a.apply_visitor(Visitor()) == 15);

  a = var{Multiply{3,4}};
  CHECK(a.apply_visitor(Visitor()) == 12);
}
