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

    void translate_and_add_equation_set(const u32string& s);

    equation_v translate_equation_set(const u32string& s);

    HD& system()
    {
      return m_interpreter;
    }

    Parser::Header& header()
    {
      return *m_header;
    }

    void loadLibrary(const u32string& s)
    {
      m_lt.loadLibrary(s, &m_interpreter);
    }

    const Tree::Expr&
    lastExpression() const
    {
      return m_lastExpr;
    }

    private:

    Parser::Header *m_header;

    Parser::ExprGrammar<Parser::iterator_t> *m_expr;
    Parser::EquationGrammar<Parser::iterator_t> *m_equation;
    Parser::TupleGrammar<Parser::iterator_t> *m_tuple;
    Parser::SkipGrammar<Parser::iterator_t> *m_skipper;

    Interpreter m_interpreter;

    ExprCompiler m_compiler;

    Libtool m_lt;

    Tree::Expr m_lastExpr;
  };
}

#endif // TRANSLATOR_HPP_INCLUDED
