#include <tl/parser_fwd.hpp>
#include <tl/expr.hpp>
#include <tl/exception.hpp>
#include <boost/format.hpp>

namespace TransLucid {

namespace Parser {

AST::Expr *insert_binary_operation(
   const BinaryOperation& op,
   AST::Expr *lhs, AST::Expr *rhs)
{
   AST::BinaryOpExpr *binop = dynamic_cast<AST::BinaryOpExpr*>(lhs);
   if (binop == 0) {
      return new AST::BinaryOpExpr(op, lhs, rhs);
   }
   if (binop->op.precedence > op.precedence) {
      return new AST::BinaryOpExpr(op, lhs, rhs);
   }
   if (binop->op.precedence < op.precedence) {
      binop->add_right(op, rhs);
      return binop;
   }
   //precedence is the same
   if (binop->op.assoc != op.assoc) {
      //error, mixed associativity of same precedence
      throw ParseError("Mixed associativity of same precedence");
   }
   if (binop->op.assoc == ASSOC_NON) {
      //error multiple non assoc operators
      throw ParseError("Multiple non associative operators of the same precedence");
   }
   if (binop->op.assoc == ASSOC_LEFT) {
      return new AST::BinaryOpExpr(op, lhs, rhs);
   }
   if (binop->op.assoc == ASSOC_RIGHT) {
      binop->add_right(op, rhs);
      return binop;
   }
   //assoc_variable or assoc_comparison
   if (binop->op != op) {
      //error multiple variadic operators
      throw ParseError("Multiple variadic operators of the same precedence");
   }
   //we have the same operator
   binop->add_leaf(rhs);
   return binop;
}

EquationHolder::EquationHolder(EquationAdder& adder)
: m_adder(adder)
{
   m_adder.setEquations(&m_equations);
}

EquationHolder::~EquationHolder() {
   m_adder.setEquations(0);
}

void printErrorMessage(file_position& pos, ParseErrorType type) {
   switch (type) {
      case Parser::error_expected_fi:
      std::cerr << formatError(pos, "expected 'fi'") << std::endl;
      break;

      case Parser::error_expected_else:
      std::cerr << formatError(pos, "expected 'else'") << std::endl;
      break;

      case Parser::error_expected_colon:
      std::cerr << formatError(pos, "expected ':'") << std::endl;
      break;

      case Parser::error_expected_dbl_semi:
      std::cerr << formatError(pos, "expected ';;'") << std::endl;
      break;

      case Parser::error_expected_primary:
      std::cerr << formatError(pos, "expected primary expression") << std::endl;
      break;

      case Parser::error_expected_close_paren:
      std::cerr << formatError(pos, "expected ')'") << std::endl;
      break;

      case Parser::error_expected_then:
      break;

      case Parser::error_expected_close_bracket:
      break;

      case error_unknown:
      std::cerr << formatError(pos, "unknown error") << std::endl;
      break;
   }
}

ustring_t formatError(const file_position& pos, const ustring_t& message)
{
   return (boost::format("%1%:%2%:%3%: error: %4%")
      % pos.file
      % pos.line
      % pos.column
      % message).str();
}

}

}
