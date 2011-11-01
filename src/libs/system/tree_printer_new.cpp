#include <tl/tree_printer.hpp>
#include <tl/tree_old_to_new.hpp>
#include <tl/output.hpp>

#include <sstream>
#include <stack>

namespace TransLucid
{

class TreePrinterNew
{
  private:
  std::ostringstream m_os;

  enum class Subtree
  {
    LEFT,
    NONE,
    RIGHT
  };

  enum class Assoc
  {
    LEFT,
    NON,
    RIGHT
  };

  enum class Precedence
  {
    MINUS_INF,
    FN_ABSTRACTION,
    WHERE_CLAUSE,
    BINARY_FN,
    FN_APP,
    PREFIX_FN,
    POSTFIX_FN
  };

  class ExprPrecedence
  {
    public:
    ExprPrecedence(int a)
    : m_a(a), m_b(0)
    {
    }

    ExprPrecedence(int a, const mpz_class& b)
    : m_a(a), m_b(b)
    {
    }

    bool
    operator<(const ExprPrecedence& rhs) const
    {
      return (m_a < rhs.m_a) || (m_a == rhs.m_a && m_b < rhs.m_b);
    }

    private:
    int m_a;

    //this allows an infinite number of precedences inside the same
    //m_a precedence
    //although really it allows binary operators to work
    mpz_class m_b;
  };

  //no attributes in visitation, so we have to keep our own stack
  //this means that every function which pushes to one of the stacks
  //MUST pop it when it exits
  std::stack<ExprPrecedence> m_precedence;
  std::stack<Assoc> m_assoc;
  std::stack<Subtree> m_subtree;

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
  operator()(bool b)
  {
    m_os << std::boolalpha << b;
  }

  void
  operator()(Special s)
  {
  }

  void
  operator()(char32_t c)
  {
  }

  void
  operator()(const u32string& s)
  {
    //probably need to escape this or something, we need to work out
    //what this all means
    m_os << "\"" << s << "\"";
  }

  void 
  operator()(const TreeNew::LiteralExpr& l)
  {
    m_os << l.type;
    operator()(l.text);
  }

  void
  operator()(const TreeNew::DimensionExpr& d)
  {
    if (d.text.size() == 0)
    {
      m_os << "dim_" << d.dim;
    }
    else
    {
      m_os << d.text;
    }
  }

  void
  operator()(const TreeNew::IdentExpr& ident)
  {
    m_os << ident.text;
  }

  void
  operator()(const TreeNew::HashSymbol&)
  {
    m_os << "#";
  }

  void
  operator()(const TreeNew::ParenExpr& p)
  {
    //pass on the paren info without modification
    apply_visitor(*this, p.e);
  }

  void
  operator()(const TreeNew::UnaryOpExpr& u)
  {
  }

  void
  operator()(const TreeNew::BinaryOpExpr& u)
  {
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
