#ifndef PARSER_HPP_INCLUDED
#define PARSER_HPP_INCLUDED

#define BOOST_PARAMETER_MAX_ARITY 10
#include <tl/parser_fwd.hpp>
#include <tl/parser_util.hpp>
#include <boost/signals2.hpp>
#include <boost/spirit/home/phoenix/bind/bind_member_function.hpp>
#include <boost/spirit/home/phoenix/statement/switch.hpp>
#include <tl/parser.hpp>
#include <tl/interpreter.hpp>

namespace TLInteractive
{

  namespace TL = TransLucid;
  namespace Signals = boost::signals2;
  namespace Spirit = TL::Parser::Spirit;
  using namespace boost::phoenix::local_names;

  class Grammar : public TL::Parser::Spirit::grammar<Grammar>
  {
    public:

    Grammar
    (
      TL::Parser::Parsers& parsers,
      TL::Parser::Header& header,
      TL::Parser::EquationAdder& adder,
      TL::Interpreter& interpreter
    )
    : parsers(parsers),
      header(header),
      adder(adder),
      interpreter(interpreter)
    {}

    TL::Parser::Parsers& parsers;
    TL::Parser::Header& header;
    TL::Parser::EquationAdder& adder;
    TL::Interpreter& interpreter;

    Signals::signal<void (std::vector<TL::AST::Expr*> const&)> postParseInput;

    template <class ScannerT>
    class definition
    {

      enum InputType
      {
        EXPRESSION,
        EQUATION
      };

      public:
      definition(Grammar const& self)
      : m_self(self),
      expect_dbl_semi(TL::Parser::error_expected_dbl_semi),
      eqnGrammar(self.header, self.parsers, self.adder),
      equationSet(self.interpreter.createEquationSet(TL::EquationGuard()))
      {
        equations = new TL::Parser::equation_v;
        self.adder.setEquations(equations);

        using boost::phoenix::delete_;
        using boost::phoenix::at;
        using boost::phoenix::pop_front;
        using boost::phoenix::switch_;
        using boost::phoenix::case_;
        using boost::phoenix::push_back;

        system =
          *(
            !(Spirit::str_p("%%") >> *eqn
            >> "%%" )
            [
              boost::phoenix::bind(&definition<ScannerT>::postParseEquations, this)
            ]
            >>
            (
             Spirit::str_p(";;")
             [
               boost::phoenix::bind(&definition<ScannerT>::postParseInput, this)
             ]
           |
             guard_parse(expr
             [
               let (_a = at(boost::phoenix::ref(self.parsers.expr_stack), 0))
               [
                  push_back(boost::phoenix::ref(exprList), _a),
                  pop_front(boost::phoenix::ref(self.parsers.expr_stack))
               ]
             ]
             ) [ TL::Parser::handle_expr_error() ]
             >>
             *(
              Spirit::str_p(";") >>
              (
                Spirit::str_p(";")
                [
                  boost::phoenix::bind(&definition<ScannerT>::postParseInput, this)
                ]
                |
                guard_parse(expr
                [
                   let (_a = at(boost::phoenix::ref(self.parsers.expr_stack), 0))
                   [
                      push_back(boost::phoenix::ref(exprList), _a),
                      pop_front(boost::phoenix::ref(self.parsers.expr_stack))
                   ]
                ]
                ) [ TL::Parser::handle_expr_error() ]
              )
              )
            )
          )
          >> Spirit::end_p;
#if 0
                  TL::Parser::handle_expr_error()
#endif

        expr = self.parsers.expr_parser.top();

        eqn = eqnGrammar >> ";;";

        BOOST_SPIRIT_DEBUG_RULE(eqn);
        BOOST_SPIRIT_DEBUG_RULE(expr);
        BOOST_SPIRIT_DEBUG_RULE(system);
        BOOST_SPIRIT_DEBUG_GRAMMAR(eqnGrammar);
      }

      TL::Parser::Spirit::rule<ScannerT> const&
      start() const
      {
        return system;
      }

      private:
      TL::Parser::Spirit::rule<ScannerT> system,
      expr,
      eqn;

      std::vector<TL::AST::Expr*> exprList;

      Grammar const& m_self;

      Spirit::guard<TL::Parser::ParseErrorType> guard_parse;

      Spirit::assertion<TL::Parser::ParseErrorType>
      expect_dbl_semi
      ;

      TL::Parser::EquationGrammar eqnGrammar;

      InputType inputType;

      void
      postParseInput()
      {
        m_self.postParseInput(exprList);

        #warning need to do something about memory,
        #warning but need to keep old demands around too
        //BOOST_FOREACH(TL::AST::Expr* e, exprList) {
        //   delete e;
        //}
        exprList.clear();
      }

      TL::Parser::equation_v* equations;
      //TL::EquationSet equationSet;

      #warning need to fix this
      void
      postParseEquations()
      {
        BOOST_FOREACH(const TL::Parser::equation_t& e, *equations)
        {
          //equationSet.addEquation
          // (TL::Equation(e.get<0>(), e.get<1>(),
          //  new TL::ASTEquation(e.get<2>())));
        }
      }
    };
  };
}

#endif // PARSER_HPP_INCLUDED
