#include <tl/variant.hpp>

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

int main()
{
  typedef TransLucid::Variant<int, A, B> var;

  var i(5);
  var a(A{4});
  var b(B{3});

  return a.apply_visitor(Visitor());
}
