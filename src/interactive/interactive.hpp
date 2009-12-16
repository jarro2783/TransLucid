#ifndef INTERACTIVE_HPP_INCLUDED
#define INTERACTIVE_HPP_INCLUDED

#include <tl/interpreter.hpp>
#include <tl/parser_fwd.hpp>
#include <tl/parser.hpp>

namespace TLInteractive {
   namespace TL = TransLucid;
   class System : public TL::Interpreter {
      public:

      System()
      :
      demands(createEquationSet(TL::EquationGuard())),
      time(0)
      {
         TL::Parser::addSymbol(
            L"demand",
            m_parseInfo.equation_names,
            m_parseInfo.equation_symbols);
      }

      void run();

      void parseHeader(const std::string& file);

      private:
      void postInputSignal(std::vector<TL::AST::Expr*> const& e);
      void postEqnSignal(TL::Parser::equation_v& eqns);
      std::string m_header;
      TL::EquationSet demands;

      size_t time;
   };
}

#endif // INTERACTIVE_HPP_INCLUDED
