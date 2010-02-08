#include <tl/interpreter.hpp>
#include <tl/constant_parser.hpp>

namespace TransLucid
{

void
Interpreter::initConstantParser()
{
  //m_constantGrammar = new Parser::ConstantGrammar(m_parsers);
  //m_parsers.constant_parser.push(*m_constantGrammar);
}

void
Interpreter::cleanupConstantParser()
{
  //delete m_constantGrammar;
}

}
