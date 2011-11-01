#include <tl/tree_printer.hpp>
#include <tl/tree_old_to_new.hpp>
#include <sstream>
#include <tl/output.hpp>

namespace TransLucid
{

class TreePrinterNew
{
  private:
  std::ostringstream m_os;

  public:
  typedef void result_type;

  const std::string
  get_string() const
  {
    return m_os.str();
  }

  template <typename T>
  void operator()(const T& e)
  {
    m_os << "not implemented";
  }

  void
  operator()(const TreeNew::IdentExpr& ident)
  {
    m_os << ident.text;
  }
};

std::string print_expr_tree_new(const Tree::Expr& expr)
{
  TreeOldToNew convert;
  TreeNew::Expr newe = boost::apply_visitor(convert, expr);

  TreePrinterNew print;
  newe.apply_visitor(print);
  return print.get_string();
}

}
