/* Parser functions.
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

#include <tl/charset.hpp>
#include <tl/exception.hpp>
#include <tl/parser_fwd.hpp>
#include <tl/parser_header_util.hpp>
#include <tl/tree_printer.hpp>

namespace TransLucid
{

namespace Parser
{

Header::Header()
{
  //add predefined dimensions
  system_dimension_symbols
    .add(chars_to_unsigned_u32string("time"), U"time")
    (chars_to_unsigned_u32string("id"), U"id")
    (chars_to_unsigned_u32string("all"), U"all")
  ;
}

std::ostream& 
operator<<(std::ostream& os, const Header& h)
{
  for (auto iter = h.loaded_libraries.begin(); 
    iter != h.loaded_libraries.end();
    ++iter
  )
  {
    os << "library ustring<" << utf32_to_utf8(*iter) << ">;;";
  }
  return os;
}

std::string
printEquation(const ParsedEquation& e)
{
  typedef std::back_insert_iterator<std::string> out_iter;
  Printer::ExprPrinter<out_iter> print_grammar;
  std::string generated;
  std::back_insert_iterator<std::string> outit(generated);

  std::string result = utf32_to_utf8(to_u32string(std::get<0>(e)));

  const Tree::Expr& guard = std::get<1>(e);
  if (boost::get<Tree::nil>(&guard) != 0)
  {
    Printer::karma::generate(outit, print_grammar, guard);
    result += " | " + generated;
  }

  const Tree::Expr& boolean = std::get<2>(e);
  if (boost::get<Tree::nil>(&boolean) != 0)
  {
    generated.clear();
    Printer::karma::generate(outit, print_grammar, boolean);
    result += " & " + generated;
  }

  result += " = ";

  generated.clear();
  Printer::karma::generate(outit, print_grammar, std::get<3>(e));
  result += generated;

  return result;
}

//I'll keep this for now because it could be of use
//TODO: clean up error handling
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
