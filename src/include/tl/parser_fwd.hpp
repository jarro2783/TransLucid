#ifndef PARSER_FWD_HPP_INCLUDED
#define PARSER_FWD_HPP_INCLUDED

#include <boost/spirit/include/classic_core.hpp>
#include <map>
#include <vector>
#include <deque>
#include <boost/spirit/include/classic_symbols.hpp>
#include <boost/spirit/include/classic_position_iterator.hpp>
#include <boost/spirit/include/classic_rule.hpp>
#include <boost/spirit/include/classic_grammar.hpp>
#include <boost/spirit/include/classic_operators.hpp>
#include <boost/spirit/include/classic_confix.hpp>
#include <boost/spirit/include/classic_stored_rule.hpp>
#include <tl/types.hpp>
#include <boost/shared_array.hpp>
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

      namespace Spirit = boost::spirit::classic;

      typedef Spirit::symbols<size_t, wchar_t> symbols_t;

      class SkipGrammar : public Spirit::grammar<SkipGrammar> {
         public:

         template <typename ScannerT>
         class definition {
            public:
            definition(SkipGrammar const& self) {
               skip = Parser::Spirit::ch_p(' ') | '\n' | '\t'
                  | Parser::Spirit::comment_p("//")
                  | Parser::Spirit::comment_p("/*", "*/");
            }

            const Spirit::rule<ScannerT>& start() const {
               return skip;
            }

            Spirit::rule<ScannerT> skip;
         };
      };

      namespace {
         SkipGrammar skip_p;
      }

      class IteratorTraits {
         public:

         typedef std::random_access_iterator_tag iterator_category;
         typedef wchar_t value_type;
         typedef std::ptrdiff_t difference_type;
         typedef value_type const* pointer;
         typedef value_type const& reference;
      };

      class IteratorBase : public IteratorTraits {
         public:
         virtual ~IteratorBase() {}

         virtual reference dereference() const = 0;
         virtual void increment() = 0;
         virtual IteratorBase *clone() const = 0;
         virtual bool equal(const IteratorBase& rhs) const = 0;
      };

      class Iterator : public IteratorTraits {
         public:
         Iterator()
         : m_backer(0)
         {}

         Iterator(const Iterator& other)
         : m_backer(other.m_backer == 0 ? 0 : other.m_backer->clone())
         {
         }

         Iterator(const IteratorBase& backer)
         : m_backer(backer.clone())
         {
         }

         ~Iterator() {
            delete m_backer;
         }

         bool operator==(const Iterator& rhs) const {
            return m_backer->equal(*rhs.m_backer);
         }

         bool operator!=(const Iterator& rhs) const {
            return !m_backer->equal(*rhs.m_backer);
         }

         Iterator& operator++() {
            m_backer->increment();
            return *this;
         }

         reference operator*() const {
            return m_backer->dereference();
         }

         Iterator& operator=(const Iterator& rhs) {
            if (this != &rhs) {
               if (rhs.m_backer) {
                  IteratorBase *backer = rhs.m_backer->clone();
                  delete m_backer;
                  m_backer = backer;
               } else {
                  delete m_backer;
                  m_backer = 0;
               }
            }
            return *this;
         }

         private:
         IteratorBase *m_backer;
      };

      class UIterator : public IteratorBase {
         public:

         UIterator()
         : m_text(0), m_length(0), m_backer(0)
         {
         }

         UIterator(const Glib::ustring& text)
         : m_length(text.length())
         {
            wchar_t *wtext = 0;
            try {
               wtext = new value_type[text.length() + 1];
               if (mbstowcs(wtext, text.c_str(), m_length+1) != m_length) {
                  throw "convert error creating iterator";
               }
               m_text.reset(wtext);
               m_backer = m_text.get();
            } catch (...) {
               delete wtext;
               throw;
            }
         }

         ~UIterator() {
         }

         bool equal(const IteratorBase& rhs) const {
            const UIterator *r = dynamic_cast<const UIterator*>(&rhs);
            if (r != 0) {
               return m_text == r->m_text && m_backer == r->m_backer;
            } else {
               return false;
            }
         }

         reference dereference() const {
            return *m_backer;
         }

         void increment() {
            ++m_backer;
         }

         UIterator make_end() const {
            UIterator end;

            end.m_text = m_text;
            if (m_text) {
               end.m_length = m_length;
               end.m_backer = m_text.get() + m_length;
            }

            return end;
         }

         UIterator *clone() const {
            UIterator *i = new UIterator(*this);
            return i;
         }

         private:
         boost::shared_array<const value_type> m_text;
         size_t m_length;
         const value_type *m_backer;
      };

      typedef Spirit::file_position_base<ustring_t> file_position;
      typedef Spirit::position_iterator<Iterator, file_position> iterator_t;
      typedef Spirit::skip_parser_iteration_policy<SkipGrammar> iter_policy_t;
      typedef Spirit::scanner_policies<iter_policy_t> scanner_policy_t;
      typedef Spirit::scanner<iterator_t, scanner_policy_t> scanner_t;
      typedef Spirit::stored_rule<scanner_t> stored_rule_t;

      class SystemGrammar;
      class ExprGrammar;
      class TupleGrammar;
      class ConstantGrammar;

      typedef boost::tuple<wstring_t, EquationGuard, AST::Expr*> equation_t;
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
            const ustring_t& op,
            const ustring_t& symbol,
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

         ustring_t op;
         ustring_t symbol;
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
            const wstring_t& type,
            gunichar start,
            gunichar end)
         : type(type), start(start), end(end)
         {}

         bool operator==(const Delimiter& rhs) const {
            return type == rhs.type && start == rhs.start && end == rhs.end;
         }

         wstring_t type;
         wchar_t start;
         wchar_t end;
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

         symbols_t delimiter_start_symbols;
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

      class ParserStack {
         public:

         #ifdef BOOST_SPIRIT_DEBUG
         ParserStack() {
            BOOST_SPIRIT_DEBUG_RULE(current);
         }
         #endif

         const Spirit::rule<scanner_t>& top() const {
            return current;
         }

         template <typename T>
         void push(const T& r) {
            rules.push(stored_rule_t(r));
            current = rules.top();
         }

         private:
         std::stack<stored_rule_t> rules;
         Spirit::rule<scanner_t> current;
      };

      struct Parsers {
         std::deque<AST::Expr*> expr_stack;
         std::deque<wstring_t> string_stack;
         ParserStack expr_parser;
         ParserStack primary_expr_parser;
         ParserStack tuple_parser;
         ParserStack constant_parser;
      };

      ustring_t formatError(const file_position& pos, const ustring_t& message);
      void printErrorMessage(file_position& pos, ParseErrorType type);
   }

}

#endif // PARSER_FWD_HPP_INCLUDED
