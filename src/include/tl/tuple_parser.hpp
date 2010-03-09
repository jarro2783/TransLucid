#ifndef TUPLE_PARSER_HPP_INCLUDED
#define TUPLE_PARSER_HPP_INCLUDED

#include <tl/parser_fwd.hpp>
#include <tl/parser_util.hpp>
#include <tl/expr.hpp>
#include <boost/spirit/home/phoenix/object/new.hpp>
#include <boost/spirit/home/phoenix/object/construct.hpp>
#include <boost/spirit/home/phoenix/container.hpp>
#include <tl/ast.hpp>

namespace TransLucid
{
  namespace Parser
  {
    typedef std::vector<boost::fusion::vector<Tree::Expr, Tree::Expr>>
      vector_pair_expr;

    template <typename Iterator>
    class TupleGrammar
    : public qi::grammar<Iterator, Tree::Expr(), SkipGrammar<Iterator>>
    {
      public:

      TupleGrammar()
      : TupleGrammar::base_type(context_perturb)
      {
        using namespace qi::labels;
        using namespace boost::phoenix;
        namespace phoenix = boost::phoenix;
        //expr = self.parsers.expr_parser.top();

        tuple_inside = pair[push_back(_val, _1)] % ',';

        pair %=
          (
             expr
           >  ':'
           > expr
          )
          //[
          //  _val = construct<boost::fusion::vector<Tree::Expr, Tree::Expr>>
          //         (_1, _2)
          //]
        ;

        context_perturb =
           (
              '['
            > tuple_inside
            > ']'
           )
           [
             _val = construct<Tree::BuildTupleExpr>(_1)
           ]
        ;

        expr.name("expr");
        pair.name("pair");
        context_perturb.name("context_perturb");
        tuple_inside.name("tuple_inside");

        BOOST_SPIRIT_DEBUG_NODE(context_perturb);
        BOOST_SPIRIT_DEBUG_NODE(pair);
        BOOST_SPIRIT_DEBUG_NODE(tuple_inside);
      }

      template <typename T>
      void
      set_expr(const T& t)
      {
        expr %= t;
      }

      private:

      qi::rule<Iterator, Tree::Expr(), SkipGrammar<Iterator>>
        expr,
        context_perturb
      ;

      qi::rule<Iterator, vector_pair_expr(), SkipGrammar<Iterator>>
        tuple_inside
      ;

      qi::rule
      <
        Iterator,
        boost::fusion::vector<Tree::Expr, Tree::Expr>(),
        SkipGrammar<Iterator>
      >
        pair
      ;
    };
  }
}

#endif // TUPLE_PARSER_HPP_INCLUDED
