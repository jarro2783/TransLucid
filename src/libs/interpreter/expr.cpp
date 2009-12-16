#include <tl/expr.hpp>

namespace TransLucid {

namespace AST {

namespace {
   mpz_class parseInteger(const ustring_t& value) {
      ustring_t::const_iterator iter = value.begin();
      if (value == "0") {
         return mpz_class();
      } else if (*iter == '0') {
         ++iter;
         int baseChar = *iter;
         int base;

         if (baseChar >= '2' && baseChar <= '9') {
            base = baseChar - '0';
         } else if (baseChar >= 'a' && baseChar <= 'z') {
            base = baseChar - 'a' + 10;
         } else if (baseChar >= 'A' && baseChar <= 'Z') {
            base = baseChar - 'A' + 36;
         } else {
            throw "invalid base in integer";
         }

         ++iter;

         return mpz_class(std::string(iter, value.end()), base);

      } else {
         return mpz_class(value.raw());
      }
   }
}

IntegerExpr::IntegerExpr(const mpz_class& value)
: m_value(value)
{
}

void BinaryOpExpr::add_right(const Parser::BinaryOperation& op, Expr *rhs) {
   size_t last = operands.size()-1;
   operands.at(last) = Parser::insert_binary_operation(op, operands.at(last), rhs);
}

void BinaryOpExpr::add_leaf(Expr *e) {
   operands.push_back(e);
}

} //namespace AST

} //namespace TransLucid
