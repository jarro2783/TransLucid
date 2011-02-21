/* Lexer tests.
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

#include <tl/lexer.hpp>
#include <tl/parser_iterator.hpp>

#define BOOST_TEST_MODULE lexer
#include <boost/test/included/unit_test.hpp>

namespace TL = TransLucid;
namespace lex = TL::Parser::lex;

BOOST_AUTO_TEST_SUITE( lexer_tests )

BOOST_AUTO_TEST_CASE ( keywords )
{
  typedef std::basic_string<wchar_t> wstring;
  typedef wstring::iterator iterator_t;

  typedef lex::lexertl::token<
    iterator_t, boost::mpl::vector<mpz_class, std::string>
  > token_type;

  typedef lex::lexertl::actor_lexer<token_type> lexer_type;
  typedef TL::Parser::lex_tl_tokens<lexer_type> tl_lexer;

  tl_lexer lexer;

  wstring input = L"ifififif0a9fifi  if   fi";

  wstring::iterator first = input.begin();
  wstring::iterator last = input.end();

  lexer_type::iterator_type iter = lexer.begin(first, last);
  lexer_type::iterator_type end = lexer.end();

  if (iter == end)
  {
    std::cerr << "ended before it even started" << std::endl;
  }

  while (iter != end && token_is_valid(*iter))
  {
    std::cout << *iter << std::endl;
    ++iter;
  }

  #if 0
  if (iter != end)
  {
    std::cout << *iter;
  }
  else
  {
    std::cout << "ended too soon";
  }

  while (iter != end)
  {
    ++iter;
  }
  #endif
}

BOOST_AUTO_TEST_SUITE_END()
