#ifndef TUPLE_PARSER_HPP_INCLUDED
#define TUPLE_PARSER_HPP_INCLUDED

#include <tl/parser_fwd.hpp>
#include <tl/parser_util.hpp>
#include <tl/expr.hpp>

namespace TransLucid {
   namespace Parser {

      class TupleGrammar : public Spirit::grammar<TupleGrammar> {

         public:
         TupleGrammar(Parsers& parsers)
         : parsers(parsers)
         {
         }

         Parsers& parsers;

         template <typename ScannerT>
         class definition {

            public:

            definition(const TupleGrammar& self)
            : expr_stack(self.parsers.expr_stack),
            expect_close_bracket(error_expected_close_bracket)
            {
               expr = self.parsers.expr_parser.top();

               tuple_inside = pair [
                  push_front(ref(list_stack), construct<std::list<AST::Expr*> >()),
                  push_back(at(ref(list_stack), 0), at(ref(expr_stack), 0)),
                  pop_front(ref(expr_stack))
               ]
               >> *(',' >> pair [
                  push_back(at(ref(list_stack), 0), at(ref(expr_stack), 0)),
                  pop_front(ref(expr_stack))
               ]
               );

               pair = (expr >> Spirit::ch_p(':') >> expr)
               [
                  let(_a = new_<AST::PairExpr>(at(ref(expr_stack), 1),
		              at(ref(expr_stack), 0)))
                  [
                     pop_front_n<2>()(ref(expr_stack)),
                     push_front(ref(expr_stack), _a)
                  ]
               ];

               context_perturb = ('['
               >> tuple_inside
               >> expect_close_bracket(Spirit::ch_p(']')))
               [
                  push_front(ref(expr_stack),
                     new_<AST::BuildTupleExpr>(
                        at(ref(list_stack), 0)
                     )
                  ),
                  pop_front(ref(list_stack))

               ]
               ;

            }

            const Spirit::rule<ScannerT>& start() const {
               return context_perturb;
            }

            private:
            Spirit::rule<ScannerT>
            context_perturb,
            pair,
            tuple_inside,
            expr
            ;

            std::deque<AST::Expr*>& expr_stack;
            std::deque<std::list<AST::Expr*> > list_stack;

            Spirit::assertion<ParseErrorType> expect_close_bracket;
         };

      };
   }
}

#endif // TUPLE_PARSER_HPP_INCLUDED
