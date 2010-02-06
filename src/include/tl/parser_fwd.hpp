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

namespace TransLucid {

   namespace AST {
      class Expr;
   }

   namespace Parser {

      enum ParseErrorType {
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

      typedef std::basic_string<wchar_t> string_type;
      typedef wchar_t char_type;
      typedef qi::symbols<char_type, std::u32string> symbols_t;

      template <typename Iterator>
      struct SkipGrammar : public qi::grammar<Iterator> {

         SkipGrammar()
         : SkipGrammar::base_type(skip)
         {
            skip = qi::char_(' ') | '\n' | '\t'
               | "//"
               | ("/*" >> *(qi::char_ - "/*") >> "*/");
         }

         qi::rule<Iterator> skip;
      };

      //class SystemGrammar;
      template <typename Iterator>
      class ExprGrammar;
      //class TupleGrammar;
      //class ConstantGrammar;

      typedef std::tuple<AST::Expr*, AST::Expr*> ParsedEquationGuard;
      typedef std::tuple<std::u32string, ParsedEquationGuard, AST::Expr*> equation_t;
      typedef std::vector<equation_t> equation_v;

      enum InfixAssoc {
         ASSOC_LEFT,
         ASSOC_RIGHT,
         ASSOC_NON,
         ASSOC_VARIABLE,
         ASSOC_COMPARISON
      };

      enum UnaryType {
         UNARY_PREFIX,
         UNARY_POSTFIX
      };

      struct BinaryOperation {

         BinaryOperation(
            InfixAssoc assoc,
            const std::u32string& op,
            const std::u32string& symbol,
            const mpz_class& precedence)
         : op(op), symbol(symbol), assoc(assoc), precedence(precedence)
         {}

         bool operator==(const BinaryOperation& rhs) const {
            return op == rhs.op && symbol == rhs.symbol &&
            assoc == rhs.assoc && precedence == rhs.precedence;
         }

         bool operator!=(const BinaryOperation& rhs) const {
            return !(*this == rhs);
         }

         std::u32string op;
         std::u32string symbol;
         InfixAssoc assoc;
         mpz_class precedence;
      };

      struct UnaryOperation {

         UnaryOperation(
            const ustring_t& op,
            const ustring_t& symbol,
            UnaryType type)
         : op(op), symbol(symbol), type(type)
         {}

         ustring_t op;
         ustring_t symbol;
         UnaryType type;

         bool operator==(const UnaryOperation& rhs) const {
            return op == rhs.op && symbol == rhs.symbol && type == rhs.type;
         }
      };

      struct Delimiter {
         Delimiter(
            const string_type& type,
            char_type start,
            char_type end)
         : type(type), start(start), end(end)
         {}

         bool operator==(const Delimiter& rhs) const {
            return type == rhs.type && start == rhs.start && end == rhs.end;
         }

         string_type type;
         char_type start;
         char_type end;
      };

      struct Header {

         Header()
         : errorCount(0)
         {
         }
         int errorCount;

         std::vector<ustring_t> dimension_names;
         symbols_t dimension_symbols;

         std::vector<ustring_t> equation_names;
         symbols_t equation_symbols;

         symbols_t binary_op_symbols;
         std::vector<BinaryOperation> binary_op_info;

         symbols_t prefix_op_symbols;
         symbols_t postfix_op_symbols;
         std::vector<UnaryOperation> unary_op_info;

         qi::symbols<char_type, Delimiter> delimiter_start_symbols;
         std::vector<Delimiter> delimiter_info;

         std::vector<ustring_t> libraries;

         bool operator==(const Header& rhs) const {
            return dimension_names == rhs.dimension_names &&
               equation_names == rhs.equation_names &&
               binary_op_info == rhs.binary_op_info &&
               unary_op_info == rhs.unary_op_info &&
               delimiter_info == rhs.delimiter_info
            ;
         }
      };

      AST::Expr *insert_binary_operation(
         const BinaryOperation& info,
         AST::Expr *lhs, AST::Expr *rhs);

      struct EquationAdder {

         template <typename C>
         struct result {
            typedef void type;
         };

         EquationAdder()
         : m_equations(0)
         {}

         void operator()(const equation_t& e) const
         {
            if (m_equations != 0) {
               m_equations->push_back(e);
            }
         }

         void setEquations(equation_v *equations) {
            m_equations = equations;
         }

         private:
         equation_v *m_equations;
      };

      struct EquationHolder {
         EquationHolder(EquationAdder& adder);
         ~EquationHolder();

         const equation_v& equations() const {
            return m_equations;
         }

         private:
         EquationHolder(const EquationHolder&);
         EquationHolder& operator=(const EquationHolder&);

         EquationAdder& m_adder;
         equation_v m_equations;
      };

   }

}

#endif // PARSER_FWD_HPP_INCLUDED
