#include <tl/expr_parser.hpp>
#include <tl/interpreter.hpp>

namespace TransLucid {

void Interpreter::initExprParser() {
   m_exprGrammar = new Parser::ExprGrammar(m_parseInfo, m_parsers);
   m_parsers.expr_parser.push(*m_exprGrammar);
}

void Interpreter::cleanupExprParser() {
   delete m_exprGrammar;
}

}
