#define BOOST_SPIRIT_DEBUG

#include <gmpxx.h>
#include <boost/spirit/include/qi_core.hpp>
#include <boost/spirit/home/phoenix/object/construct.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/home/phoenix/bind/bind_function.hpp>

namespace qi = boost::spirit::qi;

class Recursive;

typedef boost::variant
<
  bool,
  char,
  mpz_class,
  std::u32string,
  boost::recursive_wrapper<Recursive>
> Expr;

class Recursive
{
  public:
  Recursive() {}

  Recursive(const Expr& e1, const Expr& e2, char c)
  : e1(e1), e2(e2), c(c)
  {
  }

  Expr e1;
  Expr e2;
  char c;
};

namespace std
{
  ostream& operator<<(ostream& os, const std::u32string& s)
  {
    os << "u32string";
    return os;
  }

  ostream& operator<<(ostream& os, const Recursive)
  {
    os << "recursive";
    return os;
  }
}

Expr foo(const Expr& lhs, char c, const Expr& rhs)
{
  return Recursive(lhs, rhs, c);
}

template <typename Iterator>
class Grammar : public qi::grammar<Iterator, Expr(), qi::locals<Expr>>
{
  public:
  Grammar()
  : Grammar::base_type(start)
  {
    using namespace qi::labels;

    start =
       next [_a = _1]
    >> (
         *(   qi::char_('+')
           >> next
          )
          [
            _a = boost::phoenix::bind(&foo, _a, _1, _2)
          ]
       )
       [
         _val = _a
       ]
    ;

    next = qi::int_[_val = boost::phoenix::construct<mpz_class>(_1)];

    BOOST_SPIRIT_DEBUG_NODE(start);
    BOOST_SPIRIT_DEBUG_NODE(next);
  }
  
  private:
  qi::rule<Iterator, Expr(), qi::locals<Expr>> 
    start,
    next
  ;
};

int main()
{
  Grammar<std::wstring::const_iterator> g;
  return 0;
}
