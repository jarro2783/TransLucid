#ifndef CONSTANT_PARSER_HPP_INCLUDED
#define CONSTANT_PARSER_HPP_INCLUDED

#include <tl/parser_fwd.hpp>
#include <tl/parser_util.hpp>
#include <tl/expr.hpp>

namespace TransLucid {
   namespace Parser {
      class ConstantGrammar : public Spirit::grammar<ConstantGrammar> {
         public:
         ConstantGrammar(Parsers& parsers)
         : parsers(parsers)
         {}

         Parsers& parsers;

         template <typename ScannerT>
         class definition {
            public:
            definition(const ConstantGrammar& self)
            : expr_stack(self.parsers.expr_stack),
            string_stack(self.parsers.string_stack),
            angle_string(string_stack)
            {
               constant
                  =  ( identifier_p )
                     [
                        push_front(ref(string_stack), arg1)
                     ]
                     >> angle_string
                     [
                        push_front(ref(expr_stack),
                           new_<AST::ConstantExpr>(at(ref(string_stack), 1),
                           at(ref(string_stack), 0))),
                        pop_front_n<2>()(ref(string_stack))
                     ]
                  ;
            }

            const Spirit::rule<ScannerT>& start() const {
               return constant;
            }

            private:
            Spirit::rule<ScannerT> constant;

            std::deque<AST::Expr*>& expr_stack;
            std::deque<wstring_t>& string_stack;

            AngleStringGrammar angle_string;
         };
      };
   }
}

#endif // CONSTANT_PARSER_HPP_INCLUDED
