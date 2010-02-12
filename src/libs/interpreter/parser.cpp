#include <tl/parser_fwd.hpp>
#include <tl/expr.hpp>
#include <tl/exception.hpp>
#include <boost/format.hpp>

namespace TransLucid
{

namespace Parser
{

EquationHolder::EquationHolder(EquationAdder& adder)
: m_adder(adder)
{
  m_adder.setEquations(&m_equations);
}

EquationHolder::~EquationHolder()
{
  m_adder.setEquations(0);
}

#if 0
void
printErrorMessage(file_position& pos, ParseErrorType type)
{
  switch (type)
  {
    case Parser::error_expected_fi:
      std::cerr << formatError(pos, "expected 'fi'") << std::endl;
      break;

    case Parser::error_expected_else:
      std::cerr << formatError(pos, "expected 'else'") << std::endl;
      break;

    case Parser::error_expected_colon:
      std::cerr << formatError(pos, "expected ':'") << std::endl;
      break;

    case Parser::error_expected_dbl_semi:
      std::cerr << formatError(pos, "expected ';;'") << std::endl;
      break;

    case Parser::error_expected_primary:
      std::cerr << formatError(pos, "expected primary expression") << std::endl;
      break;

    case Parser::error_expected_close_paren:
      std::cerr << formatError(pos, "expected ')'") << std::endl;
      break;

    case Parser::error_expected_then:
      break;

    case Parser::error_expected_close_bracket:
      break;

    case error_unknown:
      std::cerr << formatError(pos, "unknown error") << std::endl;
      break;
  }
}
#endif

#if 0
u32string
formatError(const file_position& pos, const u32string& message)
{
  return (boost::format("%1%:%2%:%3%: error: %4%")
    % pos.file
    % pos.line
    % pos.column
    % message).str();
}
#endif

}

}
