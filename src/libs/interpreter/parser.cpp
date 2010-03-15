/* TODO: Give a descriptor.
   Copyright (C) 2009, 2010 Jarryd Beck and John Plaice

This file is part of TransLucid.

TransLucid is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 3, or (at your option)
any later version.

TransLucid is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with TransLucid; see the file COPYING.  If not see
<http://www.gnu.org/licenses/>.  */

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
