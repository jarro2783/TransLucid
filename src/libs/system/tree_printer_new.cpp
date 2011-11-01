#include <tl/tree_printer.hpp>
#include <tl/tree_old_to_new.hpp>

namespace TransLucid
{

class TreePrinterNew
{
  typedef std::string result_type;

  template <typename T>
  std::string operator()(const T& e)
  {
    return "not implemented";
  }
};

std::string print_expr_tree_new(const TreeNew::Expr& expr)
{
  
}

}
