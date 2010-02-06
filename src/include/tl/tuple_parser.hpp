#ifndef TUPLE_PARSER_HPP_INCLUDED
#define TUPLE_PARSER_HPP_INCLUDED

#include <tl/parser_fwd.hpp>
#include <tl/parser_util.hpp>
#include <tl/expr.hpp>

namespace TransLucid {
   namespace Parser {

      template <typename Iterator>
      class TupleGrammar : public qi::grammar<Iterator> {

         public:
         TupleGrammar()
         : TupleGrammar::base_type(context_perturb)
         {

            //expr = self.parsers.expr_parser.top();

            tuple_inside = pair
               >> *(',' >> pair)
            ;

            pair = (expr >> ':' >> expr)
            ;

            context_perturb =
               ('['
               >> tuple_inside
               >> ']')
            ;

         }

         qi::rule<Iterator>
            context_perturb,
            pair,
            tuple_inside,
            expr
         ;

      };
   }
}

#endif // TUPLE_PARSER_HPP_INCLUDED
