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
  #if 0
  AST::BinaryOpExpr* binop = dynamic_cast<AST::BinaryOpExpr*>(lhs);
  if (binop == 0)
  {
    return new AST::BinaryOpExpr(op, lhs, rhs);
  }
  if (binop->op.precedence > op.precedence)
  {
    return new AST::BinaryOpExpr(op, lhs, rhs);
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
    return new AST::BinaryOpExpr(op, lhs, rhs);
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
  #endif
}

} //namespace Tree

} //namespace TransLucid
