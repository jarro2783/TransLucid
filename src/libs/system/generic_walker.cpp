#include "tl/generic_walker.hpp"

namespace TransLucid
{

Tree::Expr 
GenericTreeWalker::operator()(const Tree::ParenExpr& e)
{
  return Tree::ParenExpr(
    apply_visitor(*this, e.e)
  );
}

Tree::Expr 
GenericTreeWalker::operator()(const Tree::UnaryOpExpr& e)
{
}

Tree::Expr 
GenericTreeWalker::operator()(const Tree::BinaryOpExpr& e)
{
}

Tree::Expr 
GenericTreeWalker::operator()(const Tree::IfExpr& e)
{
}

Tree::Expr 
GenericTreeWalker::operator()(const Tree::HashExpr& e)
{
  return Tree::HashExpr(apply_visitor(*this, e.e));
}

Tree::Expr 
GenericTreeWalker::operator()(const Tree::TupleExpr& e)
{
}

Tree::Expr 
GenericTreeWalker::operator()(const Tree::AtExpr& e)
{
  return Tree::AtExpr
  (
    apply_visitor(*this, e.lhs),
    apply_visitor(*this, e.rhs)
  );
}

Tree::Expr 
GenericTreeWalker::operator()(const Tree::BangExpr& e)
{
}

Tree::Expr 
GenericTreeWalker::operator()(const Tree::LambdaExpr& e)
{
}

Tree::Expr 
GenericTreeWalker::operator()(const Tree::PhiExpr& e)
{
}

Tree::Expr 
GenericTreeWalker::operator()(const Tree::BangAppExpr& e)
{
}

Tree::Expr 
GenericTreeWalker::operator()(const Tree::LambdaAppExpr& e)
{
}

Tree::Expr 
GenericTreeWalker::operator()(const Tree::PhiAppExpr& e)
{
}

Tree::Expr 
GenericTreeWalker::operator()(const Tree::WhereExpr& e)
{
}

}
