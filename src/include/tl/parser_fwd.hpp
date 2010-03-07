#ifndef PARSER_FWD_HPP_INCLUDED
#define PARSER_FWD_HPP_INCLUDED

#include <tuple>
#include <boost/spirit/include/qi_core.hpp>
#include <map>
#include <vector>
#include <deque>
//#include <boost/spirit/include/qi_position_iterator.hpp>
#include <boost/spirit/include/qi_rule.hpp>
#include <boost/spirit/include/qi_grammar.hpp>
//#include <boost/spirit/include/qi_operators.hpp>
//#include <boost/spirit/include/qi_confix.hpp>
//#include <boost/spirit/include/qi_stored_rule.hpp>
#include <tl/types.hpp>
//#include <boost/shared_array.hpp>
#include <stack>
#include <tl/equation.hpp>
#include <boost/fusion/include/adapt_struct.hpp>
#include <tl/expr.hpp>
#include <tl/ast.hpp>

namespace TransLucid
{
  namespace AST
  {
    class Expr;
  }

  namespace Parser
  {
    enum ParseErrorType
    {
      error_expected_fi,
      error_expected_else,
      error_expected_colon,
      error_expected_dbl_semi,
      error_expected_primary,
      error_expected_close_paren,
      error_expected_then,
      error_expected_close_bracket,
      //an error that hasn't been specifically dealt with
      error_unknown
    };

    namespace qi = boost::spirit::qi;
    namespace fusion = boost::fusion;

    typedef std::basic_string<wchar_t> string_type;
    typedef wchar_t char_type;
    typedef qi::symbols<char_type, std::u32string> symbols_t;

    typedef qi::standard_wide::space_type skip;

    template <typename Iterator>
    struct SkipGrammar : public qi::grammar<Iterator>
    {

      SkipGrammar()
      : SkipGrammar::base_type(skip)
      {
        skip =
          qi::char_(' ') | '\n' | '\t'
        | "//"
        | ("/*" >> *(qi::char_ - "/*") >> "*/")
        ;
      }

      qi::rule<Iterator> skip;
    };

    //class SystemGrammar;
    template <typename Iterator>
    class ExprGrammar;
    //class TupleGrammar;
    //class ConstantGrammar;

    typedef std::tuple<AST::Expr*, AST::Expr*> ParsedEquationGuard;
    typedef std::tuple<std::u32string, ParsedEquationGuard, AST::Expr*>
            equation_t;
    typedef std::vector<equation_t> equation_v;

    struct Delimiter
    {
      Delimiter() = default;

      Delimiter(
        const u32string& type,
        char_type start,
        char_type end)
      : type(type), start(start), end(end)
      {}

      bool
      operator==(const Delimiter& rhs) const
      {
        return type == rhs.type && start == rhs.start && end == rhs.end;
      }

      u32string type;
      char_type start;
      char_type end;
    };

    typedef qi::symbols<char_type, Tree::UnaryOperation> unary_symbols;
    typedef qi::symbols<char_type, Tree::BinaryOperation> binary_symbols;
    typedef qi::symbols<char_type, Delimiter> delimiter_symbols;

    struct Header
    {
      symbols_t dimension_symbols;

      binary_symbols binary_op_symbols;

      unary_symbols prefix_op_symbols;
      unary_symbols postfix_op_symbols;

      delimiter_symbols delimiter_start_symbols;

      std::vector<u32string> libraries;

      #if 0
      bool
      operator==(const Header& rhs) const
      {
        return dimension_names == rhs.dimension_names &&
          equation_names == rhs.equation_names &&
          binary_op_info == rhs.binary_op_info &&
          unary_op_info == rhs.unary_op_info &&
          delimiter_info == rhs.delimiter_info
        ;
      }
      #endif
    };

    struct EquationAdder
    {
      template <typename C>
      struct result
      {
        typedef void type;
      };

      EquationAdder()
      : m_equations(0)
      {}

      void
      operator()(const equation_t& e) const
      {
        if (m_equations != 0)
        {
          m_equations->push_back(e);
        }
      }

      void
      setEquations(equation_v* equations)
      {
        m_equations = equations;
      }

      private:
      equation_v* m_equations;
    };

    struct EquationHolder
    {
      EquationHolder(EquationAdder& adder);
      ~EquationHolder();

      const equation_v&
      equations() const
      {
        return m_equations;
      }

      private:
      EquationHolder(const EquationHolder&);
      EquationHolder& operator=(const EquationHolder&);

      EquationAdder& m_adder;
      equation_v m_equations;
    };

    typedef string_type::const_iterator iterator_t;
  }
}

#if 0
BOOST_FUSION_ADAPT_STRUCT
(
  TransLucid::Parser::Header,
  (TransLucid::Parser::symbols_t, dimension_symbols)
  (TransLucid::Parser::binary_symbols, binary_op_symbols)
  (TransLucid::Parser::unary_symbols, prefix_op_symbols)
  (TransLucid::Parser::unary_symbols, postfix_op_symbols)
  (TransLucid::Parser::delimiter_symbols, delimiter_start_symbols)
  (std::vector<std::u32string>, libraries)
)
#endif

#endif // PARSER_FWD_HPP_INCLUDED
