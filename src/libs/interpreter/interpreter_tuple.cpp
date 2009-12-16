#include <tl/tuple_parser.hpp>
#include <tl/interpreter.hpp>

namespace TransLucid {

void Interpreter::initTupleParser() {
   m_tupleGrammar = new Parser::TupleGrammar(m_parsers);
   m_parsers.tuple_parser.push(*m_tupleGrammar);
}

void Interpreter::cleanupTupleParser() {
   delete m_tupleGrammar;
}

}
