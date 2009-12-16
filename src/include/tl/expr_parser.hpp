#ifndef EXPR_PARSER_HPP_INCLUDED
#define EXPR_PARSER_HPP_INCLUDED

#include <tl/parser_util.hpp>
#include <tl/parser_fwd.hpp>
#include <tl/expr.hpp>
#include <boost/format.hpp>

namespace TransLucid {

   namespace Parser {

      using namespace boost::phoenix;
      using namespace boost::phoenix::arg_names;
      using namespace boost::phoenix::local_names;

      class ExprGrammar : public Spirit::grammar<ExprGrammar> {
         public:

         ExprGrammar(Header& h, Parsers& p)
         : header(h), parsers(p)
         {}

         Header &header;
         Parsers& parsers;

         template <typename ScannerT>
         class definition {
            public:

            definition(ExprGrammar const& self)
            : expr_stack(self.parsers.expr_stack),
            string_stack(self.parsers.string_stack),
            expect_fi(error_expected_fi),
            expect_else(error_expected_else),
            expect_colon(error_expected_colon),
            expect_primary(error_expected_primary),
            expect_close_paren(error_expected_close_paren),
            expect_then(error_expected_then),
            angle_string(string_stack)
            {
               expr = if_expr
               ;

               //stack handling for if
               //when if is seen, push a new list for elsifs
               //when elsif is seen push both expressions onto the list
               //after the whole lot, make an IfExpr with
               //the condition, the expression, the elsif list, and the else
               //expression and pop all
               if_expr =
               (
               Spirit::str_p("if")
               [
                  push_front(ref(list_stack), construct<std::list<AST::Expr*> >())
               ]
                  >> if_expr
                  >> expect_then(Spirit::str_p("then")) >> if_expr
                  >> *("elsif" >> if_expr >>
                     expect_then(Spirit::str_p("then"))
                     >> if_expr)
                  [
                     let(_a = at(ref(list_stack), 0))
                     [
                        push_back(_a, at(ref(expr_stack), 1)),
                        push_back(_a, at(ref(expr_stack), 0)),
                        pop_front_n<2>()(ref(expr_stack))
                     ]
                  ]
                  >> expect_else(Spirit::str_p("else")) >> if_expr >> expect_fi(Spirit::str_p("fi"))
               )
               [
                  let (_a = new_<AST::IfExpr>(
                     at(ref(expr_stack), 2),
                     at(ref(expr_stack), 1),
                     at(ref(list_stack), 0),
                     at(ref(expr_stack), 0)))
                  [
                     pop_front_n<3>()(ref(expr_stack)),
                     pop_front(ref(list_stack)),
                     push_front(ref(expr_stack), _a)
                  ]
               ]
               | range_expr
               ;

               range_expr
                  = at_expr
                  >> !(".."
                  >> if_expr)
                  [
                     let(_a = new_<AST::RangeExpr>(at(ref(expr_stack), 1),
                           at(ref(expr_stack), 0)))
                     [
                        pop_front_n<2>()(ref(expr_stack)),
                        push_front(ref(expr_stack), _a)
                     ]
                  ]
                  ;

               at_expr = spec_ops >> !('@' >> at_expr)
               [
                  //let(_a = at(ref(expr_stack), 1), _b = at(ref(expr_stack), 0))
                  let(_a = new_<AST::AtExpr>(at(ref(expr_stack), 1), at(ref(expr_stack), 0)))
                  [
                     pop_front_n<2>()(ref(expr_stack)),
                     push_front(ref(expr_stack), _a)
                  ]
               ]
               ;

               spec_ops = ((Spirit::str_p("isspecial") | "convert" | "istype")
                  [
                     push_front(ref(string_stack), construct<wstring_t>(arg1, arg2))
                  ]
               >> angle_string >> spec_ops)
               [
                  let(_a = new_<AST::SpecialOpsExpr>(
                     at(ref(string_stack), 1), at(ref(string_stack), 0), at(ref(expr_stack), 0)))
                     [
                        pop_front(ref(expr_stack)),
                        pop_front(ref(string_stack)),
                        push_front(ref(expr_stack), _a)
                     ]
               ]
               | binary_op
               ;

               binary_op = prefix_expr >>
               *(self.header.binary_op_symbols
               [
                  push_front(ref(op_stack), arg1)
               ]
               >> prefix_expr
               [
                  let (_a = bind(&insert_binary_operation,
                     at(ref(self.header.binary_op_info), at(ref(op_stack), 0)),
                     at(ref(expr_stack), 1),
                     at(ref(expr_stack), 0)
                  ))
                  [
                     pop_front_n<2>()(ref(expr_stack)),
                     pop_front(ref(op_stack)),
                     push_front(ref(expr_stack), _a)
                  ]
               ]
               )
               ;

               prefix_expr
                  = (self.header.prefix_op_symbols >> postfix_expr)
                  [
                     let(_a = new_<AST::UnaryExpr>(at(ref(self.header.unary_op_info), (ref(op_stack), 0)), at(ref(expr_stack), 0)))
                     [
                        pop_front(ref(expr_stack)),
                        push_back(ref(expr_stack), _a)
                     ]

                  ]
                  | postfix_expr
                  ;

               postfix_expr
                  = hash_expr >> !self.header.postfix_op_symbols
                  ;

               hash_expr =
               '#' >> hash_expr [
                  let(_a = new_<AST::HashExpr>(at(ref(expr_stack), 0))) [
                     pop_front(ref(expr_stack)),
                     push_front(ref(expr_stack), _a)
                  ]
               ]
               | expect_primary(primary_expr)
               //| primary_expr
               ;

               primary_expr
                  = integer
                  |  boolean
                  | self.header.dimension_symbols [
                     push_front(ref(expr_stack), new_<AST::DimensionExpr>(
                        at(ref(self.header.dimension_names), arg1)))
                  ]
                  | self.header.equation_symbols [
                     push_front(ref(expr_stack), new_<AST::IdentExpr>(
                        at(ref(self.header.equation_names), arg1))
                     )
                  ]
                  |   constant
                  |   context_perturb
                  |   '(' >> expr >> expect_close_paren(Spirit::ch_p(')'))
                  |   (Spirit::confix_p( self.header.delimiter_start_symbols
                  [
                     push_front(ref(string_stack),
                        bind(&getDelimiterType,
                           at(ref(self.header.delimiter_info), arg1)
                        )
                     ),
                     bind(&setEndDelimiter, at(ref(self.header.delimiter_info), arg1),
                        ref(endDelimiter))
                  ]
                  , (*Spirit::anychar_p)
                  [
                     push_front(ref(string_stack), construct<wstring_t>(arg1, arg2))
                  ]
                  ,  Spirit::ch_p(boost::ref(endDelimiter)) )
                  )
                  [
                     push_front(ref(expr_stack), new_<AST::ConstantExpr>(
                        at(ref(string_stack), 1),
                        at(ref(string_stack), 0)
                        )),
                     pop_front_n<2>()(ref(string_stack))
                  ]
                  ;

               boolean = (Spirit::str_p("true") | "false")
               [
                  push_front(ref(expr_stack),
                     new_<AST::BooleanExpr>(construct<wstring_t>(arg1, arg2)))
               ]
               ;

               integer = integer_p
               [
                  push_front(ref(expr_stack), new_<AST::IntegerExpr>(arg1))
               ]
               ;

               constant = self.parsers.constant_parser.top();

               context_perturb = self.parsers.tuple_parser.top();

               #ifdef BOOST_SPIRIT_DEBUG
               BOOST_SPIRIT_DEBUG_NODE(integer);
               BOOST_SPIRIT_DEBUG_NODE(boolean);
               BOOST_SPIRIT_DEBUG_NODE(constant);
               BOOST_SPIRIT_DEBUG_NODE(primary_expr);
               BOOST_SPIRIT_DEBUG_NODE(hash_expr);
               BOOST_SPIRIT_DEBUG_NODE(postfix_expr);
               BOOST_SPIRIT_DEBUG_NODE(prefix_expr);
               #endif
            }

            const Spirit::rule<ScannerT>& start() const {
               return expr;
            }

            private:
            Spirit::rule<ScannerT>
            if_expr,
            binary_op,
            range_expr,
            prefix_expr,
            postfix_expr,
            expr,
            at_expr,
            hash_expr,
            spec_ops,
            boolean,
            constant,
            primary_expr,
            integer,
            context_perturb
            ;

            std::deque<size_t> op_stack;
            std::deque<AST::Expr*>& expr_stack;
            std::deque<wstring_t>& string_stack;
            std::deque<std::list<AST::Expr*> > list_stack;

            //errors
            Spirit::assertion<ParseErrorType>
            expect_fi,
            expect_else,
            expect_colon,
            expect_primary,
            expect_close_paren,
            expect_then
            ;

            wchar_t endDelimiter;

            AngleStringGrammar angle_string;
         };
      };

   }

}

#endif // EXPR_PARSER_HPP_INCLUDED
