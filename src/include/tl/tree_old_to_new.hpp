#include <tl/ast.hpp>
#include <tl/ast-new.hpp>

namespace TransLucid
{
  class TreeOldToNew
  {
    public:
    typedef TreeNew::Expr result_type;

    TreeNew::Expr operator()(const Tree::nil& n)
    {
      return TreeNew::nil();
    }

    TreeNew::Expr operator()(bool b)
    {
      return b;
    }

    TreeNew::Expr operator()(Special s)
    {
      return s;
    }

    TreeNew::Expr operator()(const mpz_class& i)
    {
      return i;
    }

    TreeNew::Expr operator()(char32_t c)
    {
      return c;
    }

    TreeNew::Expr operator()(const u32string& s)
    {
      return s;
    }

    TreeNew::Expr operator()(const Tree::HashSymbol& e)
    {
      return TreeNew::HashSymbol();
    }

    TreeNew::Expr operator()(const Tree::LiteralExpr& e)
    {
      return TreeNew::LiteralExpr(e.type, e.text);
    }

    TreeNew::Expr operator()(const Tree::DimensionExpr& e)
    {
      TreeNew::DimensionExpr dim;
      dim.text = e.text;
      dim.dim = e.dim;
      return dim;
    }

    TreeNew::Expr operator()(const Tree::IdentExpr& e)
    {
      return TreeNew::IdentExpr(e.text);
    }

    TreeNew::Expr operator()(const Tree::ParenExpr& e)
    {
      return TreeNew::ParenExpr(boost::apply_visitor(*this, e.e));
    }

    TreeNew::Expr operator()(const Tree::UnaryOpExpr& e)
    {
      TreeNew::UnaryType type = TreeNew::UNARY_PREFIX;
      switch(e.op.type)
      {
        case Tree::UNARY_PREFIX:
        type = TreeNew::UNARY_PREFIX;
        break;

        case Tree::UNARY_POSTFIX:
        type = TreeNew::UNARY_POSTFIX;
        break;
      }

      return TreeNew::UnaryOpExpr
      (
        TreeNew::UnaryOperator(e.op.op, e.op.symbol, type),
        boost::apply_visitor(*this, e.e)
      );
    }

    TreeNew::Expr operator()(const Tree::BinaryOpExpr& e)
    {
      const Tree::BinaryOperator& oldop = e.op;
      TreeNew::InfixAssoc assoc = TreeNew::ASSOC_LEFT;
      switch(oldop.assoc)
      {
        case Tree::ASSOC_LEFT:
        assoc = TreeNew::ASSOC_LEFT;
        break;

        case Tree::ASSOC_RIGHT:
        assoc = TreeNew::ASSOC_RIGHT;
        break;

        case Tree::ASSOC_NON:
        assoc = TreeNew::ASSOC_NON;
        break;

        case Tree::ASSOC_VARIABLE:
        assoc = TreeNew::ASSOC_VARIABLE;
        break;

        case Tree::ASSOC_COMPARISON:
        assoc = TreeNew::ASSOC_COMPARISON;
        break;
      }

      TreeNew::BinaryOperator newop(
        assoc, oldop.op, oldop.symbol, oldop.precedence);

      return TreeNew::BinaryOpExpr
      (
        newop, 
        boost::apply_visitor(*this, e.lhs), 
        boost::apply_visitor(*this, e.rhs)
      );
    }

    TreeNew::Expr operator()(const Tree::IfExpr& e)
    {
      std::vector<std::pair<TreeNew::Expr, TreeNew::Expr>> else_ifs;

      for (const auto p : e.else_ifs)
      {
        else_ifs.push_back(std::make_pair(
          boost::apply_visitor(*this, p.first),
          boost::apply_visitor(*this, p.second)
        ));
      }

      return TreeNew::IfExpr
      (
        boost::apply_visitor(*this, e.condition),
        boost::apply_visitor(*this, e.then),
        else_ifs,
        boost::apply_visitor(*this, e.else_)
      );
    }

    TreeNew::Expr operator()(const Tree::HashExpr& e)
    {
      return TreeNew::HashExpr(boost::apply_visitor(*this, e.e));
    }

    TreeNew::Expr operator()(const Tree::TupleExpr& e);
    TreeNew::Expr operator()(const Tree::AtExpr& e);
    TreeNew::Expr operator()(const Tree::BangExpr& e);
    TreeNew::Expr operator()(const Tree::LambdaExpr& e);
    TreeNew::Expr operator()(const Tree::PhiExpr& e);
    TreeNew::Expr operator()(const Tree::BangAppExpr& e);
    TreeNew::Expr operator()(const Tree::LambdaAppExpr& e);
    TreeNew::Expr operator()(const Tree::PhiAppExpr& e);
    TreeNew::Expr operator()(const Tree::WhereExpr& e);
  };
}
