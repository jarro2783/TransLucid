#include <tl/tree_printer.hpp>
#include <tl/tree_old_to_new.hpp>

namespace TransLucid
{

class TreePrinterNew
{
  public:
  typedef std::string result_type;

  template <typename T>
  std::string operator()(const T& e)
  {
    return "not implemented";
  }
};

std::string print_expr_tree_new(const Tree::Expr& expr)
{
  TreeOldToNew convert;
  TreeNew::Expr newe = boost::apply_visitor(convert, expr);

  TreePrinterNew print;
  return newe.apply_visitor(print);
}

}
