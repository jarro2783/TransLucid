#include <tl/variant.hpp>

#define CATCH_CONFIG_MAIN
#include "catch.hpp"

struct Sum
{
  int a;
  int b;

  bool
  operator==(const Sum& rhs) const
  {
    return a == rhs.a && b == rhs.b;
  }
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

typedef TransLucid::Variant <Sum, Multiply> var;

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

TEST_CASE( "variant get", "the variant get function")
{
  var a{Sum{5,6}};
  CHECK((TransLucid::get<Sum>(a) == Sum{5,6}));

  const var b{Sum{6,7}};
  CHECK((TransLucid::get<Sum>(b) == Sum{6,7}));

  Sum* ap = TransLucid::get<Sum>(&a);
  REQUIRE(ap != 0);
  CHECK((*ap == Sum{5,6}));

  const Sum* bp = TransLucid::get<Sum>(&b);
  REQUIRE(bp != 0);
  CHECK((*bp == Sum{6,7}));
}

class Link;

typedef TransLucid::Variant <int, TransLucid::recursive_wrapper<Link>> AST;

class Link
{
  public:
  Link(const std::string& a, const AST& b)
  : x(a), y(b) {}

  std::string x;
  AST y;
};

TEST_CASE( "recursive_wrapper", "the variant with recursive wrapper" )
{
  Link l{"hello",AST{1}};
  TransLucid::recursive_wrapper<Link> lr(l);
  //TransLucid::recursive_wrapper<Link> bad(5);
  AST a(l);
  AST seven(1);
  AST b{Link{"goodbye",seven}};

  const auto& stored = TransLucid::get<Link>(b);
  CHECK(stored.x == "goodbye");
}
