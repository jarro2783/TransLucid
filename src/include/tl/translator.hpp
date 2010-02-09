#ifndef TRANSLATOR_HPP_INCLUDED
#define TRANSLATOR_HPP_INCLUDED

#include <tl/parser.hpp>
#include <tl/expr_parser.hpp>
#include <tl/hyperdaton.hpp>
#include <tl/tuple_parser.hpp>
#include <tl/interpreter.hpp>
#include <tl/expr_compiler.hpp>

namespace TransLucid
{
  class Translator
  {
    public:

    Translator();

    HD* translate_expr(const Parser::string_type& s);

    private:

    Parser::Header m_header;

    Parser::ExprGrammar<Parser::iterator_t> m_expr;
    Parser::EquationGrammar<Parser::iterator_t> m_equation;
    Parser::TupleGrammar<Parser::iterator_t> m_tuple;
    Parser::SkipGrammar<Parser::iterator_t> m_skipper;

    Interpreter m_interpreter;

    ExprCompiler m_compiler;
  };
}

#endif // TRANSLATOR_HPP_INCLUDED
