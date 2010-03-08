#include <tl/ast.hpp>

namespace TransLucid
{

namespace Tree
{

Expr
insert_binary_operation
(
  const BinaryOperation& op,
  Expr& lhs,
  Expr& rhs
)
{
  Tree::BinaryOpExpr* binop = boost::get<Tree::BinaryOpExpr>(&lhs);
  if (binop == 0)
  {
    return Tree::BinaryOpExpr(op, lhs, rhs);
  }
  if (binop->op.precedence > op.precedence)
  {
    return Tree::BinaryOpExpr(op, lhs, rhs);
  }
  if (binop->op.precedence < op.precedence)
  {
    binop->add_right(op, rhs);
    return binop;
  }
  //precedence is the same
  if (binop->op.assoc != op.assoc)
  {
    //error, mixed associativity of same precedence
    throw ParseError(U"Mixed associativity of same precedence");
  }
  if (binop->op.assoc == ASSOC_NON)
  {
    //error multiple non assoc operators
    throw
      ParseError(U"Multiple non associative operators of the same precedence");
  }
  if (binop->op.assoc == ASSOC_LEFT)
  {
    return Tree::BinaryOpExpr(op, lhs, rhs);
  }
  if (binop->op.assoc == ASSOC_RIGHT)
  {
    binop->add_right(op, rhs);
    return binop;
  }
  //assoc_variable or assoc_comparison
  if (binop->op != op)
  {
    //error multiple variadic operators
    throw ParseError(U"Multiple variadic operators of the same precedence");
  }
  //we have the same operator
  binop->add_leaf(rhs);
  return binop;
}

#warning work this out with variadic or not
void
BinaryOpExpr::add_right(const BinaryOperation& op, Expr& r)
{
  //size_t last = operands.size()-1;
  //operands.at(last) =
  //  insert_binary_operation(op, operands.at(last), r);
}

void
BinaryOpExpr::add_leaf(Expr& e)
{
  //operands.push_back(e);
  rhs = e;
}

} //namespace Tree

} //namespace TransLucid
