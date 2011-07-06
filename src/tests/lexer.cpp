/* Lexer tests.
   Copyright (C) 2011 Jarryd Beck and John Plaice

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
#include <tl/lexer.hpp>
#include <tl/lexer_util.hpp>

#include <boost/spirit/include/qi_grammar.hpp>
#include <boost/spirit/include/qi_operator.hpp>
#include <boost/spirit/include/qi_rule.hpp>
#include <boost/spirit/include/qi_lit.hpp>
#include <boost/spirit/include/qi_eoi.hpp>
#include <boost/spirit/include/qi_kleene.hpp>
#include <boost/spirit/include/qi_core.hpp>
#include <boost/spirit/include/support_istream_iterator.hpp>

#include <boost/spirit/home/phoenix/bind/bind_member_function.hpp>
#include <boost/spirit/home/phoenix/operator.hpp>
#include <boost/spirit/home/phoenix/core/reference.hpp>

#include <ostream>

#define BOOST_TEST_MODULE lexer
#include <boost/test/included/unit_test.hpp>

namespace TL = TransLucid;
namespace lex = TL::Lexer::lex;
namespace qi = boost::spirit::qi;
using TL::Lexer::value_wrapper;
using TL::Lexer::wstring;

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

ostream& operator<<(ostream& os, const pair<TL::u32string, TL::u32string>& p)
{
  os << p.first << "<" << p.second << ">";
  return os;
}

}

namespace
{
  TransLucid::System&
  getSystem()
  {
    static bool initialized = false;
    static TransLucid::System theSystem;

    if (!initialized)
    {
      theSystem.addUnaryOperator(
        TransLucid::Tree::UnaryOperator
          {U"operator+", U"+", TransLucid::Tree::UNARY_POSTFIX}
      );

      theSystem.addUnaryOperator(
        TransLucid::Tree::UnaryOperator
          {U"operator-", U"-", TransLucid::Tree::UNARY_PREFIX}
      );

      theSystem.addBinaryOperator(
        TransLucid::Tree::BinaryOperator
          {TransLucid::Tree::ASSOC_LEFT, U"operator%", U"%", 10}
      );
      initialized = true;
    }

    return theSystem;
  }

  TL::Parser::Errors errors;

  TL::Lexer::tl_lexer&
  getLexer()
  {
    static TL::Lexer::tl_lexer lexer(errors, getSystem());

    return lexer;
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
	KEYWORD_FALSE,
  KEYWORD_EQN
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
  TOKEN_DOUBLE_SEMI,
  TOKEN_DBL_PERCENT,
  TOKEN_MAPS
};

//the types of values that we can have
typedef boost::variant
<
  Keyword, 
  value_wrapper<mpz_class>, 
  value_wrapper<mpq_class>,
  value_wrapper<mpf_class>,
  TL::u32string,
  Token,
  std::pair<TL::u32string, TL::u32string>,
  char32_t,
  std::pair<TL::u32string, TL::Lexer::OpTokens>
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

  void
  error(const TL::u32string& message)
  {
    BOOST_TEST_MESSAGE("should not have reached here: " << message);
    BOOST_REQUIRE(false);
  }

	void identifier(const TL::u32string& ws)
	{
    BOOST_TEST_MESSAGE("Testing identifier: " << ws);
    BOOST_REQUIRE(m_current != m_tokens.end());

    const TL::u32string* wsp = boost::get<TL::u32string>(&*m_current);

    //if this fails the type of the token is wrong
    BOOST_REQUIRE(wsp != 0);
    BOOST_CHECK(*wsp == ws);

    ++m_current;
	}

	void integer(const value_wrapper<mpz_class>& i)
	{
    BOOST_TEST_MESSAGE("Testing integer: " << i);
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
    BOOST_TEST_MESSAGE("Testing keyword: " << t);
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
    BOOST_TEST_MESSAGE("Testing rational: " << q);
    BOOST_REQUIRE(m_current != m_tokens.end());

    const value_wrapper<mpq_class>* qp = 
      boost::get<value_wrapper<mpq_class>>(&*m_current);
    //if this fails the type of the token is wrong
    BOOST_REQUIRE(qp != 0);
    BOOST_CHECK_EQUAL(*qp, q);

    ++m_current;
  }

  void real(const value_wrapper<mpf_class>& f)
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

  void constant(const std::pair<TL::u32string, TL::u32string>& c)
  {
    BOOST_TEST_MESSAGE("Testing constant: " << c);
    BOOST_REQUIRE(m_current != m_tokens.end());

    auto cp = boost::get<std::pair<TL::u32string, TL::u32string>>(&*m_current);

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

  void
  op(const TL::u32string& text, TL::Lexer::OpTokens type)
  {
    BOOST_TEST_MESSAGE("Testing op: " << text);
    BOOST_REQUIRE(m_current != m_tokens.end());

    auto opval = boost::get<std::pair<TL::u32string, TL::Lexer::OpTokens>>
      (&*m_current);

    BOOST_REQUIRE(opval != nullptr);
    BOOST_CHECK(opval->first == text);
    BOOST_CHECK(opval->second == type);
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
        | keyword  [ph::bind(&Checker::keyword, m_checker, _1)]
        | symbol   [ph::bind(&Checker::symbol, m_checker, _1)]
        | rational
        | real
        | constant
        | tok.character_[ph::bind(&Checker::character, m_checker, _1)]
        | tok.binary_op_
          [
            ph::bind(&Checker::op, m_checker, _1, TL::Lexer::TOK_BINARY_OP)
          ]
        | tok.prefix_op_
          [
            ph::bind(&Checker::op, m_checker, _1, TL::Lexer::TOK_PREFIX_OP)
          ]
        | tok.postfix_op_
          [
            ph::bind(&Checker::op, m_checker, _1, TL::Lexer::TOK_POSTFIX_OP)
          ]
        | tok.operator_
          [
            ph::bind(&Checker::error, m_checker, TL::u32string(U"operator"))
          ]
        )
        >> qi::eoi
      ;

      ident    = tok.identifier_[ph::bind(&Checker::identifier, m_checker, _1)];
      integer  = tok.integer_   [ph::bind(&Checker::integer, m_checker, _1)];
      rational = tok.rational_  [ph::bind(&Checker::rational, m_checker, _1)];
      real     = tok.real_      [ph::bind(&Checker::real, m_checker, _1)];
      constant = (tok.constantRAW_ | tok.constantINTERPRET_)
         [
           ph::bind(&Checker::constant, m_checker, _1)
         ]
      ;

      keyword = 
        tok.if_   [_val = KEYWORD_IF] 
      | tok.fi_   [_val = KEYWORD_FI]
      | tok.where_[_val = KEYWORD_WHERE]
      | tok.then_ [_val = KEYWORD_THEN]
      | tok.elsif_[_val = KEYWORD_ELSIF]
      | tok.true_ [_val = KEYWORD_TRUE]
      | tok.false_[_val = KEYWORD_FALSE]
      | tok.eqn_  [_val = KEYWORD_EQN]
      ;

      symbol = 
        tok.dblslash_ [_val = TOKEN_DOUBLE_SLASH]
      | tok.range_    [_val = TOKEN_RANGE]
      | tok.arrow_    [_val = TOKEN_ARROW]
      | tok.dblsemi_  [_val = TOKEN_DOUBLE_SEMI]
      | tok.dblpercent_ [_val = TOKEN_DBL_PERCENT]

      | tok.decl_     [_val = TOKEN_COLON]
      | tok.lbracket_ [_val = TOKEN_OBRACKET]
      | tok.rbracket_ [_val = TOKEN_CBRACKET]
      | tok.dot_      [_val = TOKEN_DOT]
      | tok.def_      [_val = TOKEN_EQUALS]
      | tok.and_      [_val = TOKEN_AMPERSAND]
      | tok.hash_     [_val = TOKEN_HASH]
      | tok.at_       [_val = TOKEN_AT]
      | tok.slash_    [_val = TOKEN_SLASH]
      | tok.lparen_   [_val = TOKEN_OPAREN]
      | tok.rparen_   [_val = TOKEN_CPAREN]
      | tok.pipe_     [_val = TOKEN_BAR]
      | tok.maps_     [_val = TOKEN_MAPS]
      ;
    }

    qi::rule<Iterator> 
      start
    , ident
    , integer
    , rational
    , real
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
typedef checker_grammar<TL::Lexer::iterator_t> cgrammar;

bool check(const TL::u32string& input, Checker& checker)
{
  TL::System& system = getSystem();
  const TL::Tuple& t = system.getDefaultContext();
  auto& lexer = getLexer();
  lexer.m_context = &t;
  cgrammar checkg(lexer, checker);

  TL::Parser::U32Iterator first(
    TL::Parser::makeUTF32Iterator(input.begin()),
    TL::Parser::makeUTF32Iterator(input.end())
  );
  TL::Parser::U32Iterator last;

  errors.reset();

	return lex::tokenize_and_parse(first, last, lexer, checkg) && first == last
    && errors.count() == 0;
}

bool check_utf8(const std::string& input, Checker& checker)
{
  std::istringstream is(input);
  is >> std::noskipws;
  TL::Parser::U32Iterator first
  (
    TL::Parser::makeUTF8Iterator(boost::spirit::istream_iterator(is)),
    TL::Parser::makeUTF8Iterator(boost::spirit::istream_iterator())
  )
  ;
  TL::Parser::U32Iterator last;

  TL::System& system = getSystem();
  const TL::Tuple& t = system.getDefaultContext();

  auto& lexer = getLexer();
  lexer.m_context = &t;
  cgrammar checkg(lexer, checker);

  errors.reset();

  return lex::tokenize_and_parse(first, last, lexer, checkg) && first == last
    && errors.count() == 0;
}

BOOST_AUTO_TEST_CASE ( identifiers )
{
  TL::u32string input1 = U"ident";
  Checker checker1({U"ident"});
  BOOST_CHECK(check(input1, checker1));

  TL::u32string input2 
    = U"ifififif0a9fifi testing hello world a a_b a5 b abc34_";
  
  Checker checker2({
    U"ifififif0a9fifi", 
    U"testing",
    U"hello",
    U"world",
    U"a",
    U"a_b",
    U"a5",
    U"b",
    U"abc34_"
    })
  ;

  BOOST_CHECK(check(input2, checker2) == true);
}

BOOST_AUTO_TEST_CASE ( keywords )
{
  TL::u32string input = U"if fi where then elsif true false";
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
  BOOST_TEST_MESSAGE("test case: constants");
  TL::u32string input = U"`hello` 'a' '\\U00000041' '\\u0041' '\\xC2\\xA2'"
                        U"\"text\\u00E4\\xC2\\xA2\"";
  Checker checker({
    std::make_pair(U"ustring", U"hello"),
    char32_t('a'),
    U'\u0041',
    U'\u0041',
    U'\u00a2',
    std::make_pair(U"ustring", U"text\u00e4\u00a2")
  });

  check(input, checker);
}

BOOST_AUTO_TEST_CASE ( integers )
{
  BOOST_TEST_MESSAGE("test case: integers");
  TL::u32string input = U"0 1 10 50 100 021 02101 0A25 0GA 0aZJ 011 01111"
                        U" ~1 ~0Gab ~15 ~0111 ~1000"
  ;
  std::list<mpz_class> values
  ({
    0,
    1,
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
    -2,
    -1000
  });

  Checker checker(std::list<Values>(values.begin(), values.end()));

  check(input, checker);

  TL::u32string invalid = U"0AFB";
  Checker check_invalid({mpz_class(0)});
  BOOST_CHECK(check(invalid, check_invalid) == false);
}

BOOST_AUTO_TEST_CASE ( rationals )
{
  TL::u32string input = U"0_1 123_124 0GA_3 ~3_2";
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

#if 0
BOOST_AUTO_TEST_CASE ( floats )
{
  TL::u32string input = U"0.0 1.0 5.25 0GA.BC ~3.4 1.1^10 12.123456^20#500 "
    U"0G1.123456789123456789123456789123456789123456789123456789123456789"
    U"123456789#FFF";
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
#endif

BOOST_AUTO_TEST_CASE ( symbols )
{
  TL::u32string input = UR"*(: [ ] . = & # @ \ \\ .. ( ) -> | ;; \\\ %% <-)*";
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
    TOKEN_SLASH,
    TOKEN_DBL_PERCENT,
    TOKEN_MAPS
  });

  check(input, checker);
}

BOOST_AUTO_TEST_CASE ( mixed )
{
  TL::u32string input = U"intmp @ 10 (hello if) cats [1 <- 5]";
  Checker checker({
    U"intmp",
    TOKEN_AT,
    mpz_class(10),
    TOKEN_OPAREN,
    U"hello",
    KEYWORD_IF,
    TOKEN_CPAREN,
    U"cats",
    TOKEN_OBRACKET,
    mpz_class(1),
    TOKEN_MAPS,
    mpz_class(5),
    TOKEN_CBRACKET
  });

  check(input, checker);
}

BOOST_AUTO_TEST_CASE ( operators )
{
  BOOST_TEST_MESSAGE("testing the operator symbol");
  TL::u32string input = U"4 % 5 - +";
  Checker checker({
    mpz_class(4),
    std::make_pair(U"%", TL::Lexer::TOK_BINARY_OP),
    mpz_class(5),
    std::make_pair(U"-", TL::Lexer::TOK_PREFIX_OP),
    std::make_pair(U"+", TL::Lexer::TOK_POSTFIX_OP)
  });

  check(input, checker);
}

BOOST_AUTO_TEST_CASE ( utf8 )
{
  BOOST_TEST_MESSAGE("testing utf8 input stream");
  std::string input = "%% 45 4 600";
  Checker checker({
    TOKEN_DBL_PERCENT,
    mpz_class(45),
    mpz_class(4),
    mpz_class(600)
  });
  BOOST_CHECK(check_utf8(input, checker));
}

BOOST_AUTO_TEST_CASE ( comments )
{
  BOOST_TEST_MESSAGE("testing comments");

  std::string input = 
  R"(
    //comment
    eqn y = 6;; //more comments
    eqn z = 5;;
    //end comment
  )";

  Checker checker({
    KEYWORD_EQN,
    U"y",
    TOKEN_EQUALS,
    mpz_class(6),
    TOKEN_DOUBLE_SEMI,
    KEYWORD_EQN,
    U"z",
    TOKEN_EQUALS,
    mpz_class(5),
    TOKEN_DOUBLE_SEMI
  });

  check_utf8(input, checker);
}

BOOST_AUTO_TEST_SUITE_END()
