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

    TreeNew::Expr operator()(const Tree::HashSymbol& e);
    TreeNew::Expr operator()(const Tree::LiteralExpr& e);
    TreeNew::Expr operator()(const Tree::DimensionExpr& e);
    TreeNew::Expr operator()(const Tree::IdentExpr& e);
    TreeNew::Expr operator()(const Tree::ParenExpr& e);
    TreeNew::Expr operator()(const Tree::UnaryOpExpr& e);
    TreeNew::Expr operator()(const Tree::BinaryOpExpr& e);
    TreeNew::Expr operator()(const Tree::IfExpr& e);
    TreeNew::Expr operator()(const Tree::HashExpr& e);
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
