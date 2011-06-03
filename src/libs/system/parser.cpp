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

#include <tl/builtin_types.hpp>
#include <tl/charset.hpp>
#include <tl/exception.hpp>
#include <tl/parser_header.hpp>
#include <tl/tree_printer.hpp>
#include <tl/parser_util.hpp>
#include <tl/parser_api.hpp>

#include <algorithm>
#include <iterator>

namespace TransLucid
{

namespace Parser
{

namespace
{
  struct IdMapper
  {
    template <typename Value>
    auto operator()(const Value& v) 
      -> decltype(std::make_pair(v.first, Tree::Expr()))
    {
      return std::make_pair(v.first, Tree::Expr(v.second));
    }
  };
}

Header::Header()
:
  //add predefined dimensions
  system_dimension_symbols(
    {U"time",
    U"id",
    U"all"}
  )
{ 
  ReservedIdentifierMap& m = reserved_ids;;

  auto inserter = std::inserter(m, m.end());
  std::transform(
    Special::m_sv.parser_stov.begin(), 
    Special::m_sv.parser_stov.end(), 
    inserter,
    IdMapper()

    #if 0
    [](const Special::StringValueInitialiser::StringValueMap::value_type& 
       iter) 
    {
      return std::make_pair(p.first, Tree::Expr(p.second));
    } 
    #endif
  );
}

std::ostream& 
operator<<(std::ostream& os, const Header& h)
{
  os << "header";
  return os;
}

std::string
printEquation(const Equation& e)
{
  //typedef std::back_insert_iterator<std::string> out_iter;
  //Printer::ExprPrinter<out_iter> print_grammar;
  std::string generated;
  //std::back_insert_iterator<std::string> outit(generated);

  std::string result = utf32_to_utf8(to_u32string(std::get<0>(e)));

  const Tree::Expr& guard = std::get<1>(e);
  if (boost::get<Tree::nil>(&guard) == 0)
  {
    //Printer::karma::generate(outit, print_grammar, guard);
    generated = print_expr_tree(guard);
    result += " | " + generated;
  }

  const Tree::Expr& boolean = std::get<2>(e);
  if (boost::get<Tree::nil>(&boolean) == 0)
  {
    //generated.clear();
    //Printer::karma::generate(outit, print_grammar, boolean);
    generated = print_expr_tree(boolean);
    result += " & " + generated;
  }

  result += " = ";

  //generated.clear();
  //Printer::karma::generate(outit, print_grammar, std::get<3>(e));
  generated = print_expr_tree(std::get<3>(e));
  result += generated;

  return result;
}

Tree::Expr
construct_identifier
(
  const u32string& id,
  const ReservedIdentifierMap& ids
)
{
  auto iter = ids.find(id);
  if (iter == ids.end())
  {
    return Tree::IdentExpr(id);
  }
  else
  {
    return iter->second;
  }
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
