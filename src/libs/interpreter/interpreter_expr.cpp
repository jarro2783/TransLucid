#include <tl/expr_parser.hpp>

namespace TransLucid {

   namespace Parser {
      template class ExprGrammar<string_type::const_iterator>;
   }

}

#if 0
#include <tl/interpreter.hpp>

namespace TransLucid {

void Interpreter::initExprParser() {
   //m_exprGrammar = new Parser::ExprGrammar<std::u32string::const_iterator>(m_parseInfo);
   //m_parsers.expr_parser.push(*m_exprGrammar);
}

void Interpreter::cleanupExprParser() {
   //delete m_exprGrammar;
}

}

#endif
