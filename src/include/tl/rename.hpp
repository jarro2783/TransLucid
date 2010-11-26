#include <tl/ast.hpp>

namespace TransLucid
{
  class RenameIdentifier
  {
    public:

    //for boost::apply_visitor
    typedef Tree::Expr result_type;

    RenameIdentifier(const u32string& from, const u32string& to)
    : m_from(from)
    , m_to(to)
    {
    }

    template <typename T>
    Tree::Expr 
    operator()(const T& e)
    {
      return e;
    }

    Tree::Expr
    operator()(const Tree::IdentExpr& e)
    {
      if (m_from == e.text)
      {
        return Tree::IdentExpr(m_to);
      }
      else
      {
        return e;
      }
    }

    Tree::Expr
    operator()(const Tree::ParenExpr& e)
    {
      return Tree::ParenExpr(boost::apply_visitor(*this, e.e));
    }

    Tree::Expr
    rename(const Tree::Expr& e)
    {
      return boost::apply_visitor(*this, e);
    }

    private:
    u32string m_from;
    u32string m_to;
  };
}
