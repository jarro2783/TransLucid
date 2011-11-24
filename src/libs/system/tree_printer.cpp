/* Tree printer.
   Copyright (C) 2011 Jarryd Beck

This file is part of TransLucid.

TransLucid is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 3, or (at your option)
any later version.

TransLucid is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with TransLucid; see the file COPYING.  If not see
<http://www.gnu.org/licenses/>.  */

#include <sstream>
#include <stack>

#include <tl/tree_printer.hpp>
#include <tl/output.hpp>
#include <tl/parser_api.hpp>

namespace TransLucid
{

namespace Printer
{

class TreePrinterNew
{
  public:

  //you should only ever use this class through here
  //everything else needs to be public so that the visitor works,
  //but DO NOT use the visitor directly
  //I cannot be responsible for what happens if you do
  //of course since this class is only used internall, you knew all that right
  std::string printTree(const Tree::Expr& e);

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

  #if 0
  template <typename T>
  void operator()(const T& e)
  {
    m_os << "not implemented";
  }
  #endif

  void
  operator()(const Tree::nil&)
  {
    m_os << "nil";
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
  operator()(const Tree::LiteralExpr& l)
  {
    m_os << l.type;
    operator()(l.text);
  }

  void
  operator()(const Tree::DimensionExpr& d)
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
  operator()(const Tree::IdentExpr& ident)
  {
    m_os << ident.text;
  }

  void
  operator()(const Tree::HashSymbol&)
  {
    m_os << "#";
  }

  void
  operator()(const Tree::ParenExpr& p)
  {
    //pass on the paren info without modification
    apply_visitor(*this, p.e);
  }

  void
  operator()(const Tree::UnaryOpExpr& u)
  {
    Precedence precedence;
    Assoc assoc;
    Subtree subtree;
    if (u.op.type == Tree::UNARY_PREFIX)
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

    if (u.op.type == Tree::UNARY_POSTFIX)
    {
      m_os << u.op.symbol;
    }
  }

  ExprPrecedence
  build_binop_precedence(const Tree::BinaryOpExpr& binop) const
  {
    return ExprPrecedence(Precedence::BINARY_FN, binop.op.precedence);
  }

  Assoc
  binop_assoc(const Tree::BinaryOpExpr& binop) const
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
  operator()(const Tree::BinaryOpExpr& u)
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
  operator()(const Tree::IfExpr& ife)
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
  operator()(const Tree::HashExpr& h)
  {
    m_os << "#!";

    parenPush(Precedence::FN_APP, Assoc::LEFT, Subtree::RIGHT);

    apply_visitor(*this, h.e);

    parenPop();
  }

  template <typename Pair>
  void
  print_tuple_pair(const Pair& p)
  {
    apply_visitor(*this, p.first);
    m_os << " : ";
    apply_visitor(*this, p.second);
  }

  void
  operator()(const Tree::TupleExpr& t)
  {
    parenPush(Precedence::MINUS_INF, Assoc::NON, Subtree::NONE);
    m_os << "[";

    auto iter = t.pairs.begin();
    if (iter != t.pairs.end())
    {
      print_tuple_pair(*iter);
      ++iter;
    }

    while (iter != t.pairs.end())
    {
      m_os << ", ";
      print_tuple_pair(*iter);
      ++iter;
    }
    m_os << "]";
    parenPop();
  }

  void
  operator()(const Tree::AtExpr& a)
  {
    pp('(', Precedence::FN_APP);
    parenPush(Precedence::FN_APP, Assoc::LEFT, Subtree::LEFT);
    apply_visitor(*this, a.lhs);
    parenPop();

    m_os << " @ ";

    parenPush(Precedence::FN_APP, Assoc::LEFT, Subtree::RIGHT);
    apply_visitor(*this, a.rhs);
    parenPop();
    pp(')', Precedence::FN_APP);
  }

  template <typename Fn>
  void
  print_fn_abstraction(const Fn& f, const u32string& symbol)
  {
    pp('(', Precedence::FN_ABSTRACTION);
    m_os << symbol << f.name << " -> ";
    parenPush(Precedence::MINUS_INF, Assoc::NON, Subtree::NONE);
    apply_visitor(*this, f.rhs);
    parenPop();
    pp(')', Precedence::FN_ABSTRACTION);
  }

  void
  operator()(const Tree::LambdaExpr& l)
  {
    print_fn_abstraction(l, U"\\");
  }

  void
  operator()(const Tree::PhiExpr& p)
  {
    print_fn_abstraction(p, U"\\\\");
  }

  template <typename App>
  void
  print_fn_application(const App& a, char separator)
  {
    pp('(', Precedence::FN_APP);

    parenPush(Precedence::FN_APP, Assoc::LEFT, Subtree::LEFT);
    apply_visitor(*this, a.lhs);
    parenPop();

    m_os << separator;

    parenPush(Precedence::FN_APP, Assoc::LEFT, Subtree::RIGHT);
    apply_visitor(*this, a.rhs);
    parenPop();

    pp(')', Precedence::FN_APP);
  }

  void
  operator()(const Tree::BangAppExpr& b)
  {
    apply_visitor(*this, b.name);
    m_os << "!(";
    apply_visitor(*this, b.args.front());
    for (auto iter = ++b.args.begin(); iter != b.args.end(); ++iter)
    {
      m_os << ", ";
      apply_visitor(*this, *iter);
    }
    m_os << ")";
  }

  void
  operator()(const Tree::LambdaAppExpr& l)
  {
    print_fn_application(l, '.');
  }

  void
  operator()(const Tree::PhiAppExpr& l)
  {
    print_fn_application(l, ' ');
  }

  void
  operator()(const Tree::WhereExpr& w)
  {
    pp('(', Precedence::WHERE_CLAUSE);

    apply_visitor(*this, w.e);

    m_os << " where" << std::endl;

    for (const auto& v : w.vars)
    {
      std::string var = Printer::printEquation(v);
      m_os << var << ";;" << std::endl;
    }

    m_os << "end" << std::endl;

    pp(')', Precedence::WHERE_CLAUSE);
  }
};

std::string print_expr_tree(const Tree::Expr& expr)
{
  TreePrinterNew print;
  return print.printTree(expr);
}

std::string 
TreePrinterNew::printTree(const Tree::Expr& e)
{
  m_os.clear();
  parenPush(Precedence::MINUS_INF, Assoc::NON, Subtree::NONE);
  apply_visitor(*this, e);
  parenPop();
  return m_os.str();
}

std::string
printEquation(const Parser::Equation& e)
{
  std::string generated;

  std::string result = utf32_to_utf8(to_u32string(std::get<0>(e)));

  const Tree::Expr& guard = std::get<1>(e);
  if (get<Tree::nil>(&guard) == 0)
  {
    generated = print_expr_tree(guard);
    result += " " + generated;
  }

  const Tree::Expr& boolean = std::get<2>(e);
  if (get<Tree::nil>(&boolean) == 0)
  {
    generated = print_expr_tree(boolean);
    result += " | " + generated;
  }

  result += " = ";

  generated = print_expr_tree(std::get<3>(e));
  result += generated;

  return result;
}

}

}
