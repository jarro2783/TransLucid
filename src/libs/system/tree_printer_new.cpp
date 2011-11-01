#include <tl/tree_printer.hpp>
#include <tl/tree_old_to_new.hpp>
#include <tl/output.hpp>

#include <sstream>
#include <stack>

namespace TransLucid
{

class TreePrinterNew
{
  public:

  //you should only ever use this class through here
  //everything else needs to be public so that the visitor works,
  //but DO NOT use the visitor directly
  //I cannot be responsible for what happens if you do
  std::string printTree(const TreeNew::Expr& e);

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
    ExprPrecedence(Precedence a)
    : m_a(a), m_b(0)
    {
    }

    ExprPrecedence(Precedence a, const mpz_class& b)
    : m_a(a), m_b(b)
    {
    }

    bool
    operator<(const ExprPrecedence& rhs) const
    {
      return (m_a < rhs.m_a) || (m_a == rhs.m_a && m_b < rhs.m_b);
    }

    private:
    Precedence m_a;

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

  void
  parenPush(ExprPrecedence p, Assoc a, Subtree s)
  {
    m_precedence.push(p);
    m_assoc.push(a);
    m_subtree.push(s);
  }

  void 
  parenPop()
  {
    m_precedence.pop();
    m_assoc.pop();
    m_subtree.pop();
  }

  void
  do_paren_print
  (
    const ExprPrecedence& parent,
    const ExprPrecedence& mine,
    Assoc assoc,
    Subtree sub,
    char paren
  )
  {
    #ifdef DEBUG
    m_os << paren;
    #else
    if (mine < parent)
    {
      m_os << paren;
    }
    else if (!(parent < mine))
    {
      //they must be equal
      //print if different subtrees
      if (assoc == Assoc::LEFT && sub == Subtree::RIGHT)
      {
        m_os << paren;
      }
      else if (assoc == Assoc::RIGHT && sub == Subtree::LEFT)
      {
        m_os << paren;
      }
      else if (assoc == Assoc::NON && sub != Subtree::NONE)
      {
        m_os << paren;
      }
    }
    #endif
  }

  void
  pp(char c, const ExprPrecedence& mine)
  {
    do_paren_print(m_precedence.top(), 
      mine, m_assoc.top(), m_subtree.top(), c);
  }

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
  operator()(const mpz_class& z)
  {
    m_os << z;
  }

  void
  operator()(char32_t c)
  {
    u32string s(1, c);
    m_os << s;
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
    Precedence precedence;
    Assoc assoc;
    Subtree subtree;
    if (u.op.type == TreeNew::UNARY_PREFIX)
    {
      precedence = Precedence::PREFIX_FN;
      assoc = Assoc::RIGHT;
      subtree = Subtree::RIGHT;

      m_os << u.op.symbol;
    }
    else
    {
      precedence = Precedence::POSTFIX_FN;
      assoc = Assoc::LEFT;
      subtree = Subtree::LEFT;
    }

    pp('(', precedence);
    parenPush(precedence, assoc, subtree);

    apply_visitor(*this, u.e);

    parenPop();
    pp(')', precedence);

    if (u.op.type == TreeNew::UNARY_POSTFIX)
    {
      m_os << u.op.symbol;
    }
  }

  ExprPrecedence
  build_binop_precedence(const TreeNew::BinaryOpExpr& binop) const
  {
    return ExprPrecedence(Precedence::BINARY_FN, binop.op.precedence);
  }

  Assoc
  binop_assoc(const TreeNew::BinaryOpExpr& binop) const
  {
    Assoc a = Assoc::NON;

    switch(binop.op.assoc)
    {
      case Tree::ASSOC_LEFT:
      a = Assoc::LEFT;
      break;

      case Tree::ASSOC_RIGHT:
      a = Assoc::RIGHT;
      break;

      case Tree::ASSOC_NON:
      a = Assoc::NON;
      break;

      default:
      break;
    }

    return a;
  }

  void
  operator()(const TreeNew::BinaryOpExpr& u)
  {
    auto prec = build_binop_precedence(u);
    auto assoc = binop_assoc(u);

    pp('(', prec);
    parenPush(prec, assoc, Subtree::LEFT);
    apply_visitor(*this, u.lhs);
    parenPop();

    m_os << " " << u.op.symbol << " ";

    parenPush(prec, assoc, Subtree::RIGHT);
    apply_visitor(*this, u.rhs);
    parenPop();
    pp(')', prec);
  }

  void
  operator()(const TreeNew::IfExpr& ife)
  {
    m_os << "if ";
    parenPush(Precedence::MINUS_INF, Assoc::NON, Subtree::NONE);
    apply_visitor(*this, ife.condition);
    m_os << " then ";
    apply_visitor(*this, ife.then);

    for (const auto& elsif : ife.else_ifs)
    {
      m_os << " elsif ";
      apply_visitor(*this, elsif.first);
      m_os << " then ";
      apply_visitor(*this, elsif.second);
    }

    m_os << " else ";
    apply_visitor(*this, ife.else_);
    m_os << " fi";

    parenPop();
  }

  void
  operator()(const TreeNew::HashExpr& h)
  {
  }
};

std::string print_expr_tree_new(const Tree::Expr& expr)
{
  TreeOldToNew convert;
  TreeNew::Expr newe = boost::apply_visitor(convert, expr);

  TreePrinterNew print;
  return print.printTree(newe);
}

std::string 
TreePrinterNew::printTree(const TreeNew::Expr& e)
{
  m_os.clear();
  parenPush(Precedence::MINUS_INF, Assoc::NON, Subtree::NONE);
  apply_visitor(*this, e);
  parenPop();
  return m_os.str();
}

}
