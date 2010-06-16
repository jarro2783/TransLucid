#define BOOST_SPIRIT_DEBUG

#include <boost/spirit/include/qi_core.hpp>

namespace qi = boost::spirit::qi;

typedef boost::variant
<
  bool,
  char,
  std::string
> Expr;

template <typename Iterator>
class Grammar : public qi::grammar<Iterator, Expr()>
{
  public:
  Grammar()
  : Grammar::base_type(start)
  {
    start = qi::int_ >> ';';

    BOOST_SPIRIT_DEBUG_NODE(start);
  }
  
  private:
  qi::rule<Iterator, Expr()> start;
};

int main()
{
  Grammar<std::string::const_iterator> g;
  return 0;
}
