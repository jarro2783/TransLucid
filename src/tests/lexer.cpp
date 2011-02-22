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
#include <tl/charset.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/home/phoenix/bind/bind_member_function.hpp>

#define BOOST_TEST_MODULE lexer
#include <boost/test/included/unit_test.hpp>

namespace TL = TransLucid;
namespace lex = TL::Parser::lex;
namespace qi = boost::spirit::qi;

BOOST_AUTO_TEST_SUITE( lexer_tests )

// iterator type used to expose the underlying input stream
typedef std::basic_string<wchar_t> wstring;
typedef wstring::iterator base_iterator_type;

// This is the token type to return from the lexer iterator
typedef lex::lexertl::token<
	base_iterator_type, 
	boost::mpl::vector<mpz_class, wstring> 
> token_type;

// This is the lexer type to use to tokenize the input.
// We use the lexertl based lexer engine.
typedef lex::lexertl::actor_lexer<token_type> lexer_type;

typedef TL::Parser::lex_tl_tokens<lexer_type> tl_lexer;

// This is the iterator type exposed by the lexer 
typedef tl_lexer::iterator_type iterator_type;

enum Token
{
  TOKEN_IF,
	TOKEN_FI,
	TOKEN_WHERE,
	TOKEN_THEN,
	TOKEN_ELSIF,
	TOKEN_TRUE,
	TOKEN_FALSE
};

//the types of values that we can have
typedef boost::variant<Token, mpz_class, wstring> Values;
  
//checks that the tokens are correct
class Checker
{
  public:
	Checker(const std::list<Values>& tokens)
  : m_tokens(tokens)
  , m_current(m_tokens.begin())
	{
	}

	void identifier(const wstring& ws)
	{
    BOOST_REQUIRE(m_current != m_tokens.end());

    BOOST_CHECK(ws == boost::get<wstring>(*m_current));

    ++m_current;
	}

	void integer(const mpz_class& i)
	{
	  std::cerr << "int: " << i << std::endl;
	}

	void keyword(Token t)
	{
	}

	private:
  //the list of the expected tokens
  std::list<Values> m_tokens;
  std::list<Values>::const_iterator m_current;
};

template <typename Iterator>
struct checker_grammar 
  : qi::grammar<Iterator>
{
    template <typename TokenDef>
    checker_grammar(TokenDef const& tok, Checker& checker)
      : checker_grammar::base_type(start)
      , m_checker(checker)
    {
      using boost::spirit::_val;
      namespace ph = boost::phoenix;
      using boost::spirit::_1;

      start = 
        *(ident 
        | integer 
        | keyword[ph::bind(&Checker::keyword, m_checker, _1)]
        )
      ;

      ident = tok.identifier[ph::bind(&Checker::identifier, m_checker, _1)];
      integer = tok.integer; //[ph::bind(&Checker::integer, m_checker, _1)];
      keyword = 
        tok.if_[_val = TOKEN_IF] 
      | tok.fi_[_val = TOKEN_FI]
      | tok.where_[_val = TOKEN_WHERE]
      | tok.then_[_val = TOKEN_THEN]
      | tok.elsif_[_val = TOKEN_ELSIF]
      | tok.true_[_val = TOKEN_TRUE]
      | tok.false_[_val = TOKEN_TRUE]
      ;
    }

    qi::rule<Iterator> 
      start
    , ident
    , integer
		;
		qi::rule<Iterator, Token()>
      keyword
    ;

		Checker& m_checker;
};

// This is the type of the grammar to parse
typedef checker_grammar<iterator_type> cgrammar;

BOOST_AUTO_TEST_CASE ( keywords )
{
  Checker checker({
    L"ifififif0a9fifi", 
    L"testing",
    L"hello",
    L"world",
    L"a",
    L"a_b",
    L"a5",
    })
  ;
  tl_lexer lexer;
  cgrammar checkg(lexer, checker);

  wstring input = L"ifififif0a9fifi testing hello world a a_b a5";

  wstring::iterator first = input.begin();
  wstring::iterator last = input.end();

	lex::tokenize_and_parse(first, last, lexer, checkg);
}

BOOST_AUTO_TEST_SUITE_END()
