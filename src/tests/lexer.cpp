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
#include <tl/charset.hpp>
//#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/qi_grammar.hpp>
#include <boost/spirit/include/qi_operator.hpp>
#include <boost/spirit/include/qi_rule.hpp>
#include <boost/spirit/include/qi_lit.hpp>
#include <boost/spirit/include/qi_eoi.hpp>
#include <boost/spirit/include/qi_kleene.hpp>
#include <boost/spirit/include/qi_core.hpp>

#include <boost/spirit/home/phoenix/bind/bind_member_function.hpp>
#include <boost/spirit/home/phoenix/operator.hpp>
#include <boost/spirit/home/phoenix/core/reference.hpp>

#include <ostream>

#define BOOST_TEST_MODULE lexer
#include <boost/test/included/unit_test.hpp>

namespace TL = TransLucid;
namespace lex = TL::Parser::lex;
namespace qi = boost::spirit::qi;
using TL::Parser::value_wrapper;

inline std::string
to_utf8(const std::wstring& ws)
{
  return TL::utf32_to_utf8(TL::u32string(ws.begin(), ws.end()));
}

namespace std 
{
  
ostream& operator<<(ostream& os, const wstring& s)
{
  os << to_utf8(s);
  return os;
}

ostream& operator<<(ostream& os, const pair<wstring, wstring>& p)
{
  os << p.first << "<" << p.second << ">";
  return os;
}

}

BOOST_AUTO_TEST_SUITE( lexer_tests )

enum Keyword
{
  KEYWORD_IF,
	KEYWORD_FI,
	KEYWORD_WHERE,
	KEYWORD_THEN,
	KEYWORD_ELSIF,
	KEYWORD_TRUE,
	KEYWORD_FALSE
};

enum Token
{
  TOKEN_COLON,
  TOKEN_OBRACKET,
  TOKEN_CBRACKET,
  TOKEN_DOT,
  TOKEN_EQUALS,
  TOKEN_AMPERSAND,
  TOKEN_HASH,
  TOKEN_AT,
  TOKEN_SLASH,
  TOKEN_DOUBLE_SLASH,
  TOKEN_RANGE,
  TOKEN_OPAREN,
  TOKEN_CPAREN,
  TOKEN_ARROW,
  TOKEN_BAR,
  TOKEN_DOUBLE_SEMI
};

// iterator type used to expose the underlying input stream
typedef std::basic_string<wchar_t> wstring;
typedef wstring::const_iterator base_iterator_type;

// This is the token type to return from the lexer iterator
typedef lex::lexertl::token<
	base_iterator_type, 
	boost::mpl::vector
  <
    value_wrapper<mpz_class>, 
    value_wrapper<mpq_class>,
    value_wrapper<mpf_class>,
    wstring, 
    Keyword, 
    Token,
    std::pair<std::wstring, std::wstring>,
    char32_t
  > 
> token_type;

// This is the lexer type to use to tokenize the input.
// We use the lexertl based lexer engine.
typedef lex::lexertl::actor_lexer<token_type> lexer_type;

typedef TL::Parser::lex_tl_tokens<lexer_type> tl_lexer;

// This is the iterator type exposed by the lexer 
typedef tl_lexer::iterator_type iterator_type;

//the types of values that we can have
typedef boost::variant
<
  Keyword, 
  value_wrapper<mpz_class>, 
  value_wrapper<mpq_class>,
  value_wrapper<mpf_class>,
  wstring,
  Token,
  std::pair<std::wstring, std::wstring>,
  char32_t
> Values;

//checks that the tokens are correct
class Checker
{
  public:
	Checker(const std::list<Values>& tokens)
  : m_tokens(tokens)
  , m_current(m_tokens.begin())
	{
	}

  //when we clean up the checker we must have lexed everything
  ~Checker()
  {
    BOOST_CHECK(m_current == m_tokens.end());
  }

	void identifier(const wstring& ws)
	{
    std::cerr << "got identifier: " << to_utf8(ws) << std::endl;
    BOOST_REQUIRE(m_current != m_tokens.end());

    const wstring* wsp = boost::get<wstring>(&*m_current);

    //if this fails the type of the token is wrong
    BOOST_REQUIRE(wsp != 0);
    BOOST_CHECK(*wsp == ws);

    ++m_current;
	}

	void integer(const value_wrapper<mpz_class>& i)
	{
    std::cerr << "got integer: " << i << std::endl;
    BOOST_REQUIRE(m_current != m_tokens.end());

    const value_wrapper<mpz_class>* ip = 
      boost::get<value_wrapper<mpz_class>>(&*m_current);
    //if this fails the type of the token is wrong
    BOOST_REQUIRE(ip != 0);
    BOOST_CHECK_EQUAL(*ip, i);

    ++m_current;
	}

	void keyword(Keyword t)
	{
    std::cerr << "got keyword: " << t << std::endl;
    BOOST_REQUIRE(m_current != m_tokens.end());

    const Keyword* tp = boost::get<Keyword>(&*m_current);
    //if this fails the type of the token is wrong
    BOOST_REQUIRE(tp != 0);
    BOOST_CHECK_EQUAL(*tp, t);

    ++m_current;
	}

  void symbol(Token s)
  {
    BOOST_TEST_MESSAGE("Testing symbol: " << s);
    BOOST_REQUIRE(m_current != m_tokens.end());

    const Token* sp = boost::get<Token>(&*m_current);
    //if this fails the type of the token is wrong
    BOOST_REQUIRE(sp != 0);
    BOOST_CHECK_EQUAL(*sp, s);
    ++m_current;
  }

  void rational(const value_wrapper<mpq_class>& q)
  {
    std::cerr << "in rational" << std::endl;
    BOOST_TEST_MESSAGE("Testing rational: " << q);
    BOOST_REQUIRE(m_current != m_tokens.end());

    const value_wrapper<mpq_class>* qp = 
      boost::get<value_wrapper<mpq_class>>(&*m_current);
    //if this fails the type of the token is wrong
    BOOST_REQUIRE(qp != 0);
    BOOST_CHECK_EQUAL(*qp, q);

    ++m_current;
  }

  void float_val(const value_wrapper<mpf_class>& f)
  {
    BOOST_TEST_MESSAGE("Testing float: " << f);
    BOOST_REQUIRE(m_current != m_tokens.end());
    
    const value_wrapper<mpf_class>* fp = 
      boost::get<value_wrapper<mpf_class>>(&*m_current);

    //if this fails the type of the token is wrong
    BOOST_REQUIRE(fp != 0);
    //BOOST_CHECK_CLOSE(*fp, f, 0.001);
    BOOST_CHECK_EQUAL(*fp, f);

    ++m_current;
  }

  void constant(const std::pair<std::wstring, std::wstring>& c)
  {
    BOOST_TEST_MESSAGE("Testing constant: " << c);
    BOOST_REQUIRE(m_current != m_tokens.end());

    auto cp = boost::get<std::pair<std::wstring, std::wstring>>(&*m_current);

    BOOST_REQUIRE(cp != nullptr);
    BOOST_CHECK(cp->first == c.first);
    BOOST_CHECK(cp->second == c.second);
    ++m_current;
  }

  void character(char32_t c)
  {
    BOOST_TEST_MESSAGE("Testing character: " << c);
    BOOST_REQUIRE(m_current != m_tokens.end());

    auto cp = boost::get<char32_t>(&*m_current);

    BOOST_REQUIRE(cp != nullptr);
    BOOST_CHECK(c == *cp);
    ++m_current;
  }

	private:
  //the list of the expected tokens
  std::list<Values> m_tokens;
  std::list<Values>::const_iterator m_current;
};

inline void
print_no_match()
{
  std::cerr << "no token matched" << std::endl;
}

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
        *(
          ident 
        | integer
        | keyword[ph::bind(&Checker::keyword, m_checker, _1)]
        | symbol[ph::bind(&Checker::symbol, m_checker, _1)]
        | rational
        | float_val
        | constant
        | tok.character[ph::bind(&Checker::character, m_checker, _1)]
        )
        >> qi::eoi
      ;

      ident = tok.identifier[ph::bind(&Checker::identifier, m_checker, _1)];
      integer = tok.integer[ph::bind(&Checker::integer, m_checker, _1)];
      rational = tok.rational[ph::bind(&Checker::rational, m_checker, _1)];
      float_val = tok.float_val[ph::bind(&Checker::float_val, m_checker, _1)];
      constant = (tok.constant_raw | tok.constant_interpreted)
         [
           ph::bind(&Checker::constant, m_checker, _1)
         ]
      ;
      keyword = 
        tok.if_[_val = KEYWORD_IF] 
      | tok.fi_[_val = KEYWORD_FI]
      | tok.where_[_val = KEYWORD_WHERE]
      | tok.then_[_val = KEYWORD_THEN]
      | tok.elsif_[_val = KEYWORD_ELSIF]
      | tok.true_[_val = KEYWORD_TRUE]
      | tok.false_[_val = KEYWORD_FALSE]
      ;

      //using namespace TL::Parser;

      symbol = 
        qi::lit(L':')[_val = TOKEN_COLON]
      | qi::lit(L'[')[_val = TOKEN_OBRACKET]
      | qi::lit(L']')[_val = TOKEN_CBRACKET]
      | qi::lit(L'.')[_val = TOKEN_DOT]
      | qi::lit(L'=')[_val = TOKEN_EQUALS]
      | qi::lit(L'&')[_val = TOKEN_AMPERSAND]
      | qi::lit(L'#')[_val = TOKEN_HASH]
      | qi::lit(L'@')[_val = TOKEN_AT]
      | qi::lit(L"\\")[_val = TOKEN_SLASH]
      | tok.dblslash[_val = TOKEN_DOUBLE_SLASH]
      | tok.range[_val = TOKEN_RANGE]
      | qi::lit(L'(')[_val = TOKEN_OPAREN]
      | qi::lit(L')')[_val = TOKEN_CPAREN]
      | tok.arrow[_val = TOKEN_ARROW]
      | qi::lit(L'|')[_val = TOKEN_BAR]
      | tok.dblsemi[_val = TOKEN_DOUBLE_SEMI]
      ;
    }

    qi::rule<Iterator> 
      start
    , ident
    , integer
    , rational
    , float_val
    , constant
		;
		qi::rule<Iterator, Keyword()>
      keyword
    ;

    qi::rule<Iterator, Token()>
      symbol
    , multi_char_symbol
    ;

		Checker& m_checker;
};

// This is the type of the grammar to parse
typedef checker_grammar<iterator_type> cgrammar;

bool check(const wstring& input, Checker& checker)
{
  tl_lexer lexer;
  cgrammar checkg(lexer, checker);

  wstring::const_iterator first = input.begin();
  wstring::const_iterator last = input.end();

	return lex::tokenize_and_parse(first, last, lexer, checkg) && first == last;
}

BOOST_AUTO_TEST_CASE ( identifiers )
{
  wstring input = L"ifififif0a9fifi testing hello world a a_b a5 b abc34_";
  
  Checker checker({
    L"ifififif0a9fifi", 
    L"testing",
    L"hello",
    L"world",
    L"a",
    L"a_b",
    L"a5",
    L"b",
    L"abc34_"
    })
  ;

  check(input, checker);
}

BOOST_AUTO_TEST_CASE ( keywords )
{
  wstring input = L"if fi where then elsif true false";
  Checker checker({
    KEYWORD_IF,
    KEYWORD_FI,
    KEYWORD_WHERE,
    KEYWORD_THEN,
    KEYWORD_ELSIF,
    KEYWORD_TRUE,
    KEYWORD_FALSE
  });

  check(input, checker);
}

BOOST_AUTO_TEST_CASE ( constants )
{
  wstring input = L"`hello` 'a' '\\U00000041' '\\u0041' '\\xA3\\xA4'";
//                  L"\"text\\u00E4\\xA3\\xA4\"";
  Checker checker({
    std::make_pair(L"ustring", L"hello"),
    char32_t('a'),
    U'\u0041',
    U'\u0041',
    U'\u00e4'
#if 0
    std::make_pair(L"ustring", 
        [](const std::u32string& s) -> wstring 
      {return wstring(s.begin(), s.end());}
      (U"text\u00e4\xa3\xa4")
      )
#endif
  });

  check(input, checker);
}

BOOST_AUTO_TEST_CASE ( integers )
{
  wstring input = L"0 10 50 100 021 02101 0A25 0GA 0aZJ 011 01111"
                  L" ~1 ~0Gab ~15 ~0111"
  ;
  std::list<mpz_class> values
  ({
    0,
    10,
    50,
    100,
    1,
    5,
    25,
    10,
    1279,
    1,
    3,
    -1,
    -171,
    -15,
    -2
  });

  Checker checker(std::list<Values>(values.begin(), values.end()));

  check(input, checker);

  wstring invalid = L"0AFB";
  Checker check_invalid({});
  BOOST_CHECK(check(invalid, check_invalid) == false);
}

BOOST_AUTO_TEST_CASE ( rationals )
{
  wstring input = L"0_1 123_124 0GA_3 ~3_2";
  std::list<mpq_class> values
  ({
    mpq_class(),
    mpq_class(123, 124),
    mpq_class(10,3),
    mpq_class(-3,2)
  });

  Checker checker(std::list<Values>(values.begin(), values.end()));
  check(input, checker);
}

BOOST_AUTO_TEST_CASE ( floats )
{
  wstring input = L"0.0 1.0 5.25 0GA.BC ~3.4 1.1^10 12.123456^20#500 "
    L"0G1.123456789123456789123456789123456789123456789123456789123456789"
    L"123456789#FFF";
  std::list<mpf_class> values
  ({
    mpf_class(0),
    mpf_class(1),
    mpf_class(5.25),
    mpf_class("A.BC", 64, 16),
    mpf_class("-3.4"),
    mpf_class("1.1@10", 64),
    mpf_class("12.123456@20", 500),
    mpf_class("1.123456789123456789123456789123456789123456789123456789123456789123456789", 64, 16)
  });

  Checker checker(std::list<Values>(values.begin(), values.end()));
  check(input, checker);

}

BOOST_AUTO_TEST_CASE ( symbols )
{
  wstring input = L":[].=&#@\\ \\\\..()->|;;\\\\\\";
  Checker checker({
    TOKEN_COLON,
    TOKEN_OBRACKET,
    TOKEN_CBRACKET,
    TOKEN_DOT,
    TOKEN_EQUALS,
    TOKEN_AMPERSAND,
    TOKEN_HASH,
    TOKEN_AT,
    TOKEN_SLASH,
    TOKEN_DOUBLE_SLASH,
    TOKEN_RANGE,
    TOKEN_OPAREN,
    TOKEN_CPAREN,
    TOKEN_ARROW,
    TOKEN_BAR,
    TOKEN_DOUBLE_SEMI,
    TOKEN_DOUBLE_SLASH,
    TOKEN_SLASH
  });

  check(input, checker);
}

BOOST_AUTO_TEST_CASE ( mixed )
{
  wstring input = L"intmp @ 10 (hello if) cats";
  Checker checker({
    L"intmp",
    TOKEN_AT,
    mpz_class(10),
    TOKEN_OPAREN,
    L"hello",
    KEYWORD_IF,
    TOKEN_CPAREN,
    L"cats"
  });

  check(input, checker);
}

BOOST_AUTO_TEST_SUITE_END()
