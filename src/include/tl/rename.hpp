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
    operator()(const Tree::UnaryOpExpr& e)
    {
      return Tree::UnaryOpExpr(e.op, boost::apply_visitor(*this, e.e));
    }

    Tree::Expr
    operator()(const Tree::BinaryOpExpr& e)
    {
      return Tree::BinaryOpExpr
      (
        e.op,
        boost::apply_visitor(*this, e.lhs),
        boost::apply_visitor(*this, e.rhs)
      );
    }

    Tree::Expr
    operator()(const Tree::IfExpr& e)
    {
      return Tree::IfExpr
      (
        boost::apply_visitor(*this, e.condition),
        boost::apply_visitor(*this, e.then),
        rename_list(e.else_ifs),
        boost::apply_visitor(*this, e.else_)
      );
    }

    Tree::Expr
    operator()(const Tree::HashExpr& e)
    {
      return Tree::HashExpr(boost::apply_visitor(*this, e.e));
    }

    Tree::Expr
    operator()(const Tree::TupleExpr& e)
    {
      std::vector<std::pair<Tree::Expr, Tree::Expr>> renamed;

      for (auto iter = e.pairs.begin(); iter != e.pairs.end(); ++iter)
      {
        renamed.push_back(std::make_pair
        (
          boost::apply_visitor(*this, iter->first),
          boost::apply_visitor(*this, iter->second)
        ));
      }

      return renamed;
    }

    Tree::Expr
    operator()(const Tree::AtExpr& e)
    {
      return Tree::AtExpr
      (
        boost::apply_visitor(*this, e.lhs),
        boost::apply_visitor(*this, e.rhs)
      );
    }

    Tree::Expr
    rename(const Tree::Expr& e)
    {
      return boost::apply_visitor(*this, e);
    }

    std::vector<std::pair<Tree::Expr, Tree::Expr>>
    rename_list(const std::vector<std::pair<Tree::Expr, Tree::Expr>>& list)
    {
      std::vector<std::pair<Tree::Expr, Tree::Expr>> renamed;

      for (auto iter = list.begin(); iter != list.end(); ++iter)
      {
        renamed.push_back(std::make_pair
          (
            boost::apply_visitor(*this, iter->first),
            boost::apply_visitor(*this, iter->second)
          )
        );
      }

      return renamed;
    }

    private:
    u32string m_from;
    u32string m_to;
  };
}
