#ifndef TL_PARSER_INCLUDED
#define TL_PARSER_INCLUDED

#include <list>
#include <map>
#include <boost/foreach.hpp>
#include <tl/expr.hpp>
#include <deque>
#include <boost/format.hpp>
#include <wchar.h>
#include <boost/shared_array.hpp>
#include <tl/exception.hpp>

//#include <boost/spirit/include/phoenix_core.hpp>

#include <tl/parser_util.hpp>
#include <tl/parser_fwd.hpp>

namespace TransLucid {
   namespace Parser {

      //typedef Spirit::position_iterator<std::string::const_iterator> iterator_t;

      //T must be a boost tuple
      template <int N>
      struct get_tuple_n {

         template <typename Arg1>
         struct result {
            typedef typename boost::tuples::element<N, Arg1>::type type;
         };

         template <typename T>
         typename boost::tuples::element<N, T>::type
         operator()(const T& t) const {
            return t.template get<N>();
         }
      };

      inline void addDimensions(
         const std::vector<ustring_t>& dims,
         Spirit::symbols<>& dimsyms)
      {
         int i = dims.size();
         BOOST_FOREACH(const ustring_t& s, dims) {
            dimsyms.add(s.begin(), s.end(), i);
            ++i;
         }
      }

      inline void addSymbol(const wstring_t& symbol,
         std::vector<ustring_t>& names,
         symbols_t& symbols)
      {
         size_t *i = Spirit::add(symbols, symbol.c_str(), names.size());
         if (i) {
            names.push_back(symbol);
         }
      }

      inline void addOpDefinition(
         Header& header,
         InfixAssoc assoc,
         const wstring_t& op,
         const wstring_t& symbol,
         AST::Expr *precedence)
      {
         size_t pos = header.binary_op_info.size();
         if (symbol.size() == 1) {
            Spirit::add(header.binary_op_symbols, symbol.c_str(), pos);
         }
         wstring_t underscoreSymbol = L"_" + symbol + L"_";
         Spirit::add(header.binary_op_symbols, underscoreSymbol.c_str(), pos);
         AST::IntegerExpr *p = dynamic_cast<AST::IntegerExpr*>(precedence);
         header.binary_op_info.push_back(BinaryOperation(assoc, op, symbol, p->m_value));
      }

      inline void addDelimiter(
         Header& header,
         const wstring_t& type,
         const wstring_t& open,
         const wstring_t& close)
      {
         //std::cout << "adding " << open << " " << close << std::endl;
         if (!Spirit::add(
            header.delimiter_start_symbols,
            open.c_str(),
            header.delimiter_info.size()))
         {
            throw ParseError("open delimiter '" + ustring_t(open) + "' already defined");
         } else {
            header.delimiter_info.push_back(Delimiter(type, open[0], close[0]));
         }
      }

      class HeaderGrammar : public Spirit::grammar<HeaderGrammar> {
         public:

         HeaderGrammar(
            Header& header,
            Parsers& parsers)
         : header(header), parsers(parsers)
         {
         }

         Header& header;
         Parsers& parsers;

         template <typename ScannerT>
         class definition {
            public:
            definition(const HeaderGrammar& self)
            //initialise errors
            : self(self),
               expr_stack(self.parsers.expr_stack),
               string_stack(self.parsers.string_stack),
               expect_dbl_semi(error_expected_dbl_semi),
               angle_string(string_stack)
            {
               assoc_symbols.add("infixl", ASSOC_LEFT)
               ("infixr", ASSOC_RIGHT)
               ("infixn", ASSOC_NON)
               ("infixp", ASSOC_COMPARISON)
               ("infixm", ASSOC_VARIABLE)
               ;

               header
                  = *( headerItem >> Spirit::str_p( ";;" )) >> Spirit::end_p;
                  ;

               headerItem
                  =  Spirit::str_p("dimension")
                     >> angle_string
                     [
                        //push_back(ref(self.header.dimension_names), at(ref(string_stack), 0)),
                        bind(&addSymbol, at(ref(string_stack), 0),
                           ref(self.header.dimension_names),
                           ref(self.header.dimension_symbols)),
                        pop_front(ref(string_stack))
                     ]
                     | ( assoc_symbols
                     [
                        ref(currentAssoc) = arg1
                     ]
                     >> "ustring"
                     >> angle_string
                     >> "ustring"
                     >> angle_string
                     >> integer )
                     [
                        bind(&addOpDefinition,
                           ref(self.header),
                           ref(currentAssoc),
                           at(ref(string_stack), 1),
                           at(ref(string_stack), 0),
                           at(ref(expr_stack), 0)),
                        pop_front_n<2>()(ref(string_stack)),
                        pop_front(ref(expr_stack))
                     ]
                     | ( Spirit::str_p("delimiters")
                     >> "ustring"
                     >> angle_string
                     >> "uchar"
                     >> angle_string
                     >> "uchar"
                     >> angle_string)
                     [
                        bind(&addDelimiter, ref(self.header),
                           at(ref(string_stack), 2),
                           at(ref(string_stack), 1),
                           at(ref(string_stack), 0)
                           ),
                        pop_front_n<3>()(ref(string_stack))
                     ]
                     | ( Spirit::str_p("library")
                     >> "ustring"
                     >> angle_string )
                     [
                        push_back(ref(self.header.libraries),
                           at(ref(string_stack), 0)),
                        pop_front(ref(string_stack))
                     ]
                     | ( Spirit::str_p("prefix") | "postfix" )
                     >> "ustring"
                     >> angle_string
                     >> "ustring"
                     >> angle_string
                  ;

               integer = integer_p[push_front(ref(expr_stack),
                  new_<AST::IntegerExpr>(arg1))];

               constant = self.parsers.constant_parser.top();

               BOOST_SPIRIT_DEBUG_RULE(constant);
               BOOST_SPIRIT_DEBUG_GRAMMAR(angle_string);
               BOOST_SPIRIT_DEBUG_RULE(header);
               BOOST_SPIRIT_DEBUG_RULE(headerItem);
               BOOST_SPIRIT_DEBUG_RULE(integer);
            }

            const Spirit::rule<ScannerT>& start() const {
               return header;
            }

            ~definition() {
               //delete anything left over in the stacks because of errors

               BOOST_FOREACH(std::list<AST::Expr*>& l, list_stack) {
                  std::for_each(l.begin(), l.end(), delete_(arg1));
               }
            }

            private:

            const HeaderGrammar& self;

            Spirit::rule<ScannerT>
            header,
            headerItem,
            constant,
            integer
            ;

            std::deque<AST::Expr*>& expr_stack;
            std::deque<wstring_t>& string_stack;
            std::deque<std::list<AST::Expr*> > list_stack;

            Spirit::symbols<InfixAssoc> assoc_symbols;
            InfixAssoc currentAssoc;

            Spirit::assertion<ParseErrorType> expect_dbl_semi;

            AngleStringGrammar angle_string;
         };
      };

      class EquationGrammar : public Spirit::grammar<EquationGrammar> {
         public:

         EquationGrammar(Header& header,
            Parsers& parsers,
            EquationAdder& adder)
         : header(header),
         parsers(parsers),
         adder(adder)
         {}

         Header& header;
         Parsers& parsers;
         EquationAdder& adder;

         template <typename ScannerT>
         class definition {
            public:

            definition(EquationGrammar const& self)
            : expr_stack(self.parsers.expr_stack),
            string_stack(self.parsers.string_stack),
            expect_dbl_semi(error_expected_dbl_semi)
            {
               context_perturb = self.parsers.tuple_parser.top();

               equation = ( identifier_p
               [
                  push_front(ref(string_stack), arg1)
               ]
               >>
               !( (Spirit::ch_p('@') >> context_perturb) |
                  Spirit::epsilon_p[push_front(ref(expr_stack), (AST::Expr*)0)])
               >>
               !( (Spirit::ch_p('|') >> expr) |
                  Spirit::epsilon_p[push_front(ref(expr_stack), (AST::Expr*)0)])
               >> '=' >> expr)
               [
                  bind(self.adder,
                     construct<equation_t>(
                        at(ref(string_stack), 0),
                        construct<EquationGuard>(
                           at(ref(expr_stack), 2),
                           at(ref(expr_stack), 1)),
                        at(ref(expr_stack), 0))
                     ),
                  bind(&addSymbol,
                     at(ref(string_stack), 0),
                     ref(self.header.equation_names),
                     ref(self.header.equation_symbols)
                  ),
                  pop_front(ref(string_stack)),
                  pop_front_n<3>()(ref(expr_stack))
               ]
               ;

               expr = self.parsers.expr_parser.top();
            }

            Spirit::rule<ScannerT> const& start() const {
               return equation;
            }

            private:
            std::deque<AST::Expr*>& expr_stack;
            std::deque<wstring_t>& string_stack;

            Spirit::rule<ScannerT> equation,
            context_perturb,
            expr;

            Spirit::assertion<ParseErrorType> expect_dbl_semi;
         };
      };
   }
}

#endif
