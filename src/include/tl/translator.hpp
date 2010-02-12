#ifndef TRANSLATOR_HPP_INCLUDED
#define TRANSLATOR_HPP_INCLUDED

#include <tl/hyperdaton.hpp>
#include <tl/interpreter.hpp>
#include <tl/expr_compiler.hpp>
#include <tl/parser_fwd.hpp>

namespace TransLucid
{
  namespace Parser
  {
    class Header;

    template <typename Iterator>
    class ExprGrammar;

    template <typename Iterator>
    class EquationGrammar;

    template <typename Iterator>
    class TupleGrammar;

    template <typename Iterator>
    class SkipGrammar;
  }

  class Translator
  {
    public:

    Translator();

    HD* translate_expr(const Parser::string_type& s);

    void translate_equation_set(const u32string& s);

    HD* translate_equation(const u32string& s);

    private:

    Parser::Header *m_header;

    Parser::ExprGrammar<Parser::iterator_t> *m_expr;
    Parser::EquationGrammar<Parser::iterator_t> *m_equation;
    Parser::TupleGrammar<Parser::iterator_t> *m_tuple;
    Parser::SkipGrammar<Parser::iterator_t> *m_skipper;

    Interpreter m_interpreter;

    ExprCompiler m_compiler;
  };
}

#endif // TRANSLATOR_HPP_INCLUDED
