#ifndef TUPLE_PARSER_HPP_INCLUDED
#define TUPLE_PARSER_HPP_INCLUDED

#include <tl/parser_fwd.hpp>
#include <tl/parser_util.hpp>
#include <tl/expr.hpp>
#include <boost/spirit/home/phoenix/object/new.hpp>
#include <boost/spirit/home/phoenix/object/construct.hpp>
#include <boost/spirit/home/phoenix/container.hpp>

namespace TransLucid
{
  namespace Parser
  {
    template <typename Iterator>
    class TupleGrammar : public qi::grammar<Iterator, AST::Expr*(), SkipGrammar<Iterator>>
    {
      public:

      TupleGrammar()
      : TupleGrammar::base_type(context_perturb)
      {
        using namespace qi::labels;
        using namespace boost::phoenix;
        namespace phoenix = boost::phoenix;
        //expr = self.parsers.expr_parser.top();

        tuple_inside =
          (pair % ',')
          [
            _val = new_<AST::BuildTupleExpr>(_1)
          ]
        ;

        pair =
          (
              expr
           >  ':'
           > expr
          )
          [
            _val = construct<boost::fusion::vector<AST::Expr*, AST::Expr*>>
                   (_1, _2)
          ]
        ;

        context_perturb =
           (
              '['
            > tuple_inside
            > ']'
           )
           [_val = _1]
        ;

        expr.name("expr");
        pair.name("pair");
        context_perturb.name("context_perturb");
        tuple_inside.name("tuple_inside");
      }

      template <typename T>
      void
      set_expr(const T& t)
      {
        expr = t;
      }

      private:

      qi::rule<Iterator, AST::Expr*(), SkipGrammar<Iterator>>
        context_perturb,
        expr,
        tuple_inside
      ;

      qi::rule<Iterator, boost::fusion::vector<AST::Expr*, AST::Expr*>(), SkipGrammar<Iterator>>
        pair
      ;
    };
  }
}

#endif // TUPLE_PARSER_HPP_INCLUDED
