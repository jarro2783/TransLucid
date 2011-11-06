/* Lexertl tests.
   Copyright (C) 2011 Jarryd Beck

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

#include "tl/lexertl.hpp"

#include <tl/output.hpp>
#include <tl/system.hpp>

#include <list>

#define CATCH_CONFIG_MAIN
#include "catch.hpp"

namespace TL = TransLucid;

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

}

enum Keyword
{
  KEYWORD_ASSIGN,
  KEYWORD_DIM,
  KEYWORD_IF,
	KEYWORD_FI,
  KEYWORD_IN,
  KEYWORD_OUT,
  KEYWORD_END,
	KEYWORD_WHERE,
	KEYWORD_THEN,
	KEYWORD_ELSIF,
	KEYWORD_ELSE,
	KEYWORD_TRUE,
	KEYWORD_FALSE,
  KEYWORD_VAR
};

enum Token
{
  TOKEN_BANG,
  TOKEN_COLONEQUALS,
  TOKEN_COLON,
  TOKEN_COMMA,
  TOKEN_LSQUARE,
  TOKEN_RSQUARE,
  TOKEN_LBRACE,
  TOKEN_RBRACE,
  TOKEN_DOT,
  TOKEN_EQUALS,
  TOKEN_HASH,
  TOKEN_AT,
  TOKEN_SLASH,
  TOKEN_DOUBLE_SLASH,
  TOKEN_RANGE,
  TOKEN_LPAREN,
  TOKEN_RPAREN,
  TOKEN_LARROW,
  TOKEN_PIPE,
  TOKEN_DOUBLE_SEMI,
  TOKEN_DBL_PERCENT,
  TOKEN_RARROW
};

//the types of values that we can have
typedef TL::Variant
<
  Keyword, 
  mpz_class, 
  TL::u32string,
  Token,
  std::pair<TL::u32string, TL::u32string>,
  char32_t,
  std::pair<TL::u32string, int>
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
    CHECK(m_current == m_tokens.end());
  }

  void
  error(const TL::u32string& message)
  {
    FAIL("should not have reached here: " << message);
  }

	void identifier(const TL::u32string& ws)
	{
    INFO("Testing identifier: " << ws);
    REQUIRE(m_current != m_tokens.end());

    const TL::u32string* wsp = TL::get<TL::u32string>(&*m_current);

    //if this fails the type of the token is wrong
    REQUIRE(wsp != 0);
    CHECK(*wsp == ws);

    ++m_current;
	}

	void integer(const mpz_class& i)
	{
    INFO("Testing integer: " << i);
    REQUIRE(m_current != m_tokens.end());

    const mpz_class* ip = 
      TL::get<mpz_class>(&*m_current);
    //if this fails the type of the token is wrong
    REQUIRE(ip != 0);
    CHECK(*ip == i);

    ++m_current;
	}

	void keyword(Keyword t)
	{
    INFO("Testing keyword: " << t);
    REQUIRE(m_current != m_tokens.end());

    const Keyword* tp = TL::get<Keyword>(&*m_current);
    //if this fails the type of the token is wrong
    REQUIRE(tp != 0);
    CHECK(*tp == t);

    ++m_current;
	}

  void symbol(Token s)
  {
    INFO("Testing symbol: " << s);
    REQUIRE(m_current != m_tokens.end());

    const Token* sp = TL::get<Token>(&*m_current);
    //if this fails the type of the token is wrong
    REQUIRE(sp != 0);
    CHECK(*sp == s);
    ++m_current;
  }

#if 0
  void rational(const value_wrapper<mpq_class>& q)
  {
    INFO("Testing rational: " << q);
    REQUIRE(m_current != m_tokens.end());

    const value_wrapper<mpq_class>* qp = 
      TL::get<value_wrapper<mpq_class>>(&*m_current);
    //if this fails the type of the token is wrong
    REQUIRE(qp != 0);
    CHECK(*qp == q);

    ++m_current;
  }

  void real(const value_wrapper<mpf_class>& f)
  {
    BOOST_TEST_MESSAGE("Testing float: " << f);
    REQUIRE(m_current != m_tokens.end());
    
    const value_wrapper<mpf_class>* fp = 
      TL::get<value_wrapper<mpf_class>>(&*m_current);

    //if this fails the type of the token is wrong
    REQUIRE(fp != 0);
    //CHECK_CLOSE(*fp, f, 0.001);
    CHECK_EQUAL(*fp, f);

    ++m_current;
  }
#endif

  void constant(const std::pair<TL::u32string, TL::u32string>& c)
  {
    INFO("Testing constant: " << c);
    REQUIRE(m_current != m_tokens.end());

    auto cp = TL::get<std::pair<TL::u32string, TL::u32string>>(&*m_current);

    REQUIRE(cp != nullptr);
    CHECK(cp->first == c.first);
    CHECK(cp->second == c.second);
    ++m_current;
  }

  void character(char32_t c)
  {
    INFO("Testing character: " << c);
    REQUIRE(m_current != m_tokens.end());

    auto cp = TL::get<char32_t>(&*m_current);

    REQUIRE(cp != nullptr);
    CHECK(c == *cp);
    ++m_current;
  }

  void
  op(const TL::u32string& text, int type)
  {
    INFO("Testing op: " << text);
    REQUIRE(m_current != m_tokens.end());

    auto opval = TL::get<std::pair<TL::u32string, int>>
      (&*m_current);

    REQUIRE(opval != nullptr);
    CHECK(opval->first == text);
    CHECK(opval->second == type);
    ++m_current;
  }

  void
  where(const TL::u32string& label)
  {
    INFO("Testing where: " << label);
    REQUIRE(m_current != m_tokens.end());

    auto opval = TL::get<TL::u32string>(&*m_current);

    REQUIRE(opval != nullptr);
    CHECK(*opval == label);
    ++m_current;
  }

	private:
  //the list of the expected tokens
  std::list<Values> m_tokens;
  std::list<Values>::const_iterator m_current;
};

typedef TL::Parser::PositionIterator<TL::Parser::U32Iterator> iterator;

bool
parse
(
  iterator& begin,
  iterator& end,
  TL::Context& context,
  TL::System::IdentifierLookup& idents,
  Checker& checker
)
{
  bool success = true;

  while (begin != end && success)
  {
    auto tok = TL::Parser::nextToken(begin, end, context, idents);

    try
    {
    
      switch(tok.getType())
      {
        //symbols
        case TL::Parser::TOKEN_ASSIGNTO: //:=
        checker.symbol(TOKEN_COLONEQUALS);
        break;

        case TL::Parser::TOKEN_AT:
        checker.symbol(TOKEN_AT);
        break;

        case TL::Parser::TOKEN_BANG:
        checker.symbol(TOKEN_BANG);
        break;

        case TL::Parser::TOKEN_COLON:
        checker.symbol(TOKEN_COLON);
        break;

        case TL::Parser::TOKEN_COMMA:
        checker.symbol(TOKEN_COMMA);
        break;

        case TL::Parser::TOKEN_DBLPERCENT:
        checker.symbol(TOKEN_DBL_PERCENT);
        break;

        case TL::Parser::TOKEN_DBLSEMI:
        checker.symbol(TOKEN_DOUBLE_SEMI);
        break;

        case TL::Parser::TOKEN_DBLSLASH:
        checker.symbol(TOKEN_DOUBLE_SLASH);
        break;

        case TL::Parser::TOKEN_DOT:
        checker.symbol(TOKEN_DOT);
        break;

        case TL::Parser::TOKEN_EQUALS:
        checker.symbol(TOKEN_EQUALS);
        break;

        case TL::Parser::TOKEN_HASH:
        checker.symbol(TOKEN_HASH);
        break;

        case TL::Parser::TOKEN_LARROW:
        checker.symbol(TOKEN_LARROW);
        break;

        case TL::Parser::TOKEN_LBRACE:
        checker.symbol(TOKEN_LBRACE);
        break;

        case TL::Parser::TOKEN_LPAREN:
        checker.symbol(TOKEN_LPAREN);
        break;

        case TL::Parser::TOKEN_LSQUARE:
        checker.symbol(TOKEN_LSQUARE);
        break;

        case TL::Parser::TOKEN_PIPE:
        checker.symbol(TOKEN_PIPE);
        break;

        case TL::Parser::TOKEN_RARROW:
        checker.symbol(TOKEN_RARROW);
        break;

        case TL::Parser::TOKEN_RBRACE:
        checker.symbol(TOKEN_RBRACE);
        break;

        case TL::Parser::TOKEN_RPAREN:
        checker.symbol(TOKEN_RPAREN);
        break;

        case TL::Parser::TOKEN_RSQUARE:
        checker.symbol(TOKEN_RSQUARE);
        break;

        case TL::Parser::TOKEN_SLASH:
        checker.symbol(TOKEN_SLASH);
        break;

        break;

        //keywords
        case TL::Parser::TOKEN_ASSIGN: //assign
        checker.keyword(KEYWORD_ASSIGN);
        break;

        case TL::Parser::TOKEN_DIM:
        checker.keyword(KEYWORD_DIM);
        break;

        case TL::Parser::TOKEN_FI:
        checker.keyword(KEYWORD_FI);
        break;

        case TL::Parser::TOKEN_IF:
        checker.keyword(KEYWORD_IF);
        break;

        case TL::Parser::TOKEN_IN:
        checker.keyword(KEYWORD_IN);
        break;

        case TL::Parser::TOKEN_THEN:
        checker.keyword(KEYWORD_THEN);
        break;

        case TL::Parser::TOKEN_OUT:
        checker.keyword(KEYWORD_OUT);
        break;

        case TL::Parser::TOKEN_VAR:
        checker.keyword(KEYWORD_VAR);
        break;

        case TL::Parser::TOKEN_ELSE:
        checker.keyword(KEYWORD_ELSE);
        break;

        case TL::Parser::TOKEN_ELSIF:
        checker.keyword(KEYWORD_ELSIF);
        break;

        case TL::Parser::TOKEN_END:
        checker.keyword(KEYWORD_END);
        break;

        case TL::Parser::TOKEN_FALSE:
        checker.keyword(KEYWORD_FALSE);
        break;

        case TL::Parser::TOKEN_TRUE:
        checker.keyword(KEYWORD_TRUE);
        break;

        break;

        //constants
        case TL::Parser::TOKEN_CONSTANT_RAW:
        case TL::Parser::TOKEN_CONSTANT_INTERPRETED:
        checker.constant(TL::get<std::pair<TL::u32string, TL::u32string>>
          (tok.getValue()));
        break;

        case TL::Parser::TOKEN_UCHAR:
        checker.character(TL::get<char32_t>(tok.getValue()));
        break;


        //identifier
        case TL::Parser::TOKEN_ID:
        checker.identifier(TL::get<TL::u32string>(tok.getValue()));
        break;

        //infix declaration
        case TL::Parser::TOKEN_INFIXBIN:
        break;

        //unary symbol declaration
        case TL::Parser::TOKEN_UNARY:
        checker.identifier(TL::get<TL::u32string>(tok.getValue()));
        break;

        //integer
        case TL::Parser::TOKEN_INTEGER:
        checker.integer(TL::get<mpz_class>(tok.getValue()));
        break;
        
        //ops
        case TL::Parser::TOKEN_PREFIX_OP:
        case TL::Parser::TOKEN_BINARY_OP:
        case TL::Parser::TOKEN_POSTFIX_OP:
        checker.op(TL::get<TL::u32string>(tok.getValue()), tok.getType());
        break;

        //where
        case TL::Parser::TOKEN_WHERE:
        checker.where(TL::get<TL::u32string>(tok.getValue()));
        break;

        case TL::Parser::TOKEN_RANGE:
        default:
        success = false;
        break;
      }
    }
    catch(...)
    {
      success = false;
    }
  }
  return success;
}

bool check(const TL::u32string& input, Checker& checker)
{
  TL::System& system = getSystem();
  TL::Context& k = system.getDefaultContext();
  TL::System::IdentifierLookup idents = system.lookupIdentifiers();

  iterator first
  (
    TL::Parser::U32Iterator(
      TL::Parser::makeUTF32Iterator(input.begin()),
      TL::Parser::makeUTF32Iterator(input.end())
    ),
    input
  );
  iterator last(TL::Parser::U32Iterator{});

  return parse(first, last, k, idents, checker)
    && first == last;
}

bool check_utf8(const std::string& input, Checker& checker)
{
  TL::System& system = getSystem();
  TL::Context& k = system.getDefaultContext();
  TL::System::IdentifierLookup idents = system.lookupIdentifiers();

  iterator first
  (
    TL::Parser::U32Iterator(
      TL::Parser::makeUTF8Iterator(input.begin()),
      TL::Parser::makeUTF8Iterator(input.end())
    ),
    TL::utf8_to_utf32(input)
  );
  iterator last(TL::Parser::U32Iterator{});

  return parse(first, last, k, idents, checker)
    && first == last;
}

TEST_CASE ( "identifiers", "check that identifiers are lexed" )
{
  TL::u32string input1 = U"ident";
  Checker checker1({U"ident"});
  CHECK(check(input1, checker1));

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

  CHECK(check(input2, checker2) == true);
}

TEST_CASE ( "keywords", "check that the keywords are recognised" )
{
  TL::u32string input = U"if fi where then elsif true false";
  Checker checker({
    KEYWORD_IF,
    KEYWORD_FI,
    TL::u32string(),
    KEYWORD_THEN,
    KEYWORD_ELSIF,
    KEYWORD_TRUE,
    KEYWORD_FALSE
  });

  check(input, checker);
}

TEST_CASE ( "constants", "check the built in constants" )
{
  INFO("test case: constants");
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

TEST_CASE ( "integers", "check the integers" )
{
  INFO("test case: integers");
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

  //TL::u32string invalid = U"0AFB";
  //Checker check_invalid({mpz_class(0)});
  //CHECK(check(invalid, check_invalid) == false);
}

#if 0
TEST_CASE ( "rationals", "check the rational numbers" )
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

TEST_CASE ( floats )
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

TEST_CASE ( "symbols", "check all the symbols" )
{
  TL::u32string input = UR"*(: [ ] . = # @ \ \\ .. ( ) -> | ;; \\\ %% <-)*";
  //TL::u32string input = UR"*(: [ ] . = # @ ( ) -> | ;; %% <-)*";
  Checker checker({
    TOKEN_COLON,
    TOKEN_LSQUARE,
    TOKEN_RSQUARE,
    TOKEN_DOT,
    TOKEN_EQUALS,
    TOKEN_HASH,
    TOKEN_AT,
    TOKEN_SLASH,
    TOKEN_DOUBLE_SLASH,
    std::make_pair(U"..", TL::Parser::TOKEN_BINARY_OP),
    TOKEN_LPAREN,
    TOKEN_RPAREN,
    TOKEN_RARROW,
    TOKEN_PIPE,
    TOKEN_DOUBLE_SEMI,
    TOKEN_DOUBLE_SLASH,
    TOKEN_SLASH,
    TOKEN_DBL_PERCENT,
    TOKEN_LARROW
  });

  check(input, checker);
}

TEST_CASE ( "mixed", "random test" )
{
  TL::u32string input = U"intmp @ 10 (hello if) cats [1 <- 5]";
  Checker checker({
    U"intmp",
    TOKEN_AT,
    mpz_class(10),
    TOKEN_LPAREN,
    U"hello",
    KEYWORD_IF,
    TOKEN_RPAREN,
    U"cats",
    TOKEN_LSQUARE,
    mpz_class(1),
    TOKEN_LARROW,
    mpz_class(5),
    TOKEN_RSQUARE
  });

  check(input, checker);
}

TEST_CASE ( "operators", "check arbitrary operators" )
{
  INFO("testing the operator symbol");
  TL::u32string input = U"4 % 5 - +";
  Checker checker({
    mpz_class(4),
    std::make_pair(U"%", TL::Parser::TOKEN_BINARY_OP),
    mpz_class(5),
    std::make_pair(U"-", TL::Parser::TOKEN_PREFIX_OP),
    std::make_pair(U"+", TL::Parser::TOKEN_POSTFIX_OP)
  });

  check(input, checker);
}

TEST_CASE ( "utf8", "utf8 input stream" )
{
  INFO("testing utf8 input stream");
  std::string input = "%% 45 4 600";
  Checker checker({
    TOKEN_DBL_PERCENT,
    mpz_class(45),
    mpz_class(4),
    mpz_class(600)
  });
  CHECK(check_utf8(input, checker));
}

TEST_CASE ( "comments", "check that comments work" )
{
  INFO("testing comments");

  std::string input = 
  R"(
    //comment
    var y = 6;; //more comments
    var z = 5;;
    //end comment
  )";

  Checker checker({
    KEYWORD_VAR,
    U"y",
    TOKEN_EQUALS,
    mpz_class(6),
    TOKEN_DOUBLE_SEMI,
    KEYWORD_VAR,
    U"z",
    TOKEN_EQUALS,
    mpz_class(5),
    TOKEN_DOUBLE_SEMI
  });

  check_utf8(input, checker);
}
