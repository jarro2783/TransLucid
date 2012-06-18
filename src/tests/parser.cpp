/* Parser tests.
   Copyright (C) 2011, 2012 Jarryd Beck

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

#include "tl/parser.hpp"
#include <tl/tree_printer.hpp>
#include "tl/lexertl.hpp"
#include <tl/line_tokenizer.hpp>
#include <tl/output.hpp>

#define CATCH_CONFIG_MAIN
#include "catch.hpp"

namespace TL = TransLucid;

TEST_CASE ( "empty input", "empty input to line tokenizer" )
{
  std::string input;

  TL::Parser::U32Iterator iter(
    TL::Parser::makeUTF8Iterator(input.begin())
  );

  TL::Parser::U32Iterator end(
    TL::Parser::makeUTF8Iterator(input.end())
  );

  TL::LineTokenizer tokenize(iter, end);

  auto n = tokenize.next();

  CHECK(n.type == TL::LineType::EMPTY);
  CHECK(n.text == TL::u32string());
}

TEST_CASE ( "simple", "simply line iterator tests" )
{
  std::string input = "eqn a = 5;; assign y := 6;;";

  TL::Parser::U32Iterator iter(
    TL::Parser::makeUTF8Iterator(input.begin())
  );

  TL::Parser::U32Iterator end(
    TL::Parser::makeUTF8Iterator(input.end())
  );

  TL::LineTokenizer tokenize(iter, end);

  auto n = tokenize.next();
  CHECK(n.type == TL::LineType::LINE);
  CHECK(n.text == TL::u32string(U"eqn a = 5;;"));

  n = tokenize.next();
  CHECK(n.type == TL::LineType::LINE);
  CHECK(n.text == TL::u32string(U"assign y := 6;;"));
}

TEST_CASE( "dollar symbol", "line tokenizer $$" )
{
  std::string input = "eqn a = b;;  $$";

  TL::Parser::U32Iterator iter(TL::Parser::makeUTF8Iterator(input.begin()));
  TL::Parser::U32Iterator end(
    TL::Parser::makeUTF8Iterator(input.end())
  );

  TL::LineTokenizer tokenize(iter, end);

  auto n = tokenize.next();

  CHECK(n.type == TL::LineType::LINE);
  CHECK(n.text == TL::u32string(U"eqn a = b;;"));

  n = tokenize.next();
  CHECK(n.type == TL::LineType::DOUBLE_DOLLAR);
  CHECK(n.text == TL::u32string(U"$$"));

  n = tokenize.next();
  CHECK(n.type == TL::LineType::EMPTY);
  CHECK(n.text == TL::u32string());
}

TEST_CASE( "white space", "line tokenizer white space" )
{
  std::string input = "eqn a = b;;  $$  ";

  TL::Parser::U32Iterator iter(
    TL::Parser::makeUTF8Iterator(input.begin())
  );
  TL::Parser::U32Iterator end(
    TL::Parser::makeUTF8Iterator(input.end())
  );

  TL::LineTokenizer tokenize(iter, end);

  auto n = tokenize.next();
  CHECK(n.type == TL::LineType::LINE);
  CHECK(n.text == TL::u32string(U"eqn a = b;;"));

  n = tokenize.next();
  CHECK(n.type == TL::LineType::DOUBLE_DOLLAR);
  CHECK(n.text == TL::u32string(U"$$"));

  n = tokenize.next();
  CHECK(n.type == TL::LineType::EMPTY);
  CHECK(n.text == TL::u32string());
}

TEST_CASE( "%%", "line tokenizer %%" )
{
  std::string input = "eqn x = 42;;\n%%\nx;;";

  TL::Parser::U32Iterator iter(
    TL::Parser::makeUTF8Iterator(input.begin())
  );
  TL::Parser::U32Iterator end(
    TL::Parser::makeUTF8Iterator(input.end())
  );

  TL::LineTokenizer tokenize(iter, end);

  auto n = tokenize.next();
  CHECK(n.type == TL::LineType::LINE);
  CHECK(n.text == TL::u32string(U"eqn x = 42;;"));

  n = tokenize.next();
  CHECK(n.type == TL::LineType::DOUBLE_PERCENT);
  CHECK(n.text == TL::u32string(U"%%"));

  n = tokenize.next();
  CHECK(n.type == TL::LineType::LINE);
  CHECK(n.text == TL::u32string(U"x;;"));

  n = tokenize.next();
  CHECK(n.type == TL::LineType::EMPTY);
  CHECK(n.text == TL::u32string());
}

TEST_CASE( "where clause", "does a where clause with vars work" )
{
  {
    std::string input = "x where var x = 5;; end;;";

    TL::Parser::U32Iterator iter(
      TL::Parser::makeUTF8Iterator(input.begin())
    );
    TL::Parser::U32Iterator end(
      TL::Parser::makeUTF8Iterator(input.end())
    );

    TL::LineTokenizer tokenize(iter, end);

    auto n = tokenize.next();
    CHECK(n.type == TL::LineType::LINE);
    CHECK(n.text == TL::to_u32string(input));
  }

  {
    std::string input = 
R"(var f = x + #d where
   var x = 5;;
   dim d <- 4;;
end;;)";

    
    TL::Parser::U32Iterator iter(
      TL::Parser::makeUTF8Iterator(input.begin())
    );
    TL::Parser::U32Iterator end(
      TL::Parser::makeUTF8Iterator(input.end())
    );

    TL::LineTokenizer tokenize(iter, end);

    auto n = tokenize.next();
    CHECK(n.type == TL::LineType::LINE);
    CHECK(n.text == TL::to_u32string(input));
  }

}

TEST_CASE( "comment", "where clause with comments" )
{
  {
    std::string input_no_comment = 
R"(var f = x + #d where
   var x = 5;;
   dim d <- 4;;
end;;)";
    
    std::string input_comment = "//this is a comment\n" + input_no_comment;

    
    TL::Parser::U32Iterator iter(
      TL::Parser::makeUTF8Iterator(input_comment.begin())
    );
    TL::Parser::U32Iterator end(
      TL::Parser::makeUTF8Iterator(input_comment.end())
    );

    TL::LineTokenizer tokenize(iter, end);

    auto n = tokenize.next();
    CHECK(n.type == TL::LineType::LINE);
    CHECK(n.text == TL::to_u32string(input_no_comment));
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
          {U"plus", U"+", TransLucid::Tree::UNARY_POSTFIX}
      );

      theSystem.addUnaryOperator(
        TransLucid::Tree::UnaryOperator
          {U"minus", U"-", TransLucid::Tree::UNARY_PREFIX}
      );

      theSystem.addBinaryOperator(
        TransLucid::Tree::BinaryOperator
          {TransLucid::Tree::ASSOC_LEFT, U"modulus", U"%", 10}
      );

      theSystem.addBinaryOperator(
        TransLucid::Tree::BinaryOperator
          {TransLucid::Tree::ASSOC_LEFT, U"range_construct", U"..", 0}
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
  TOKEN_EOF,
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
  TOKEN_LPAREN,
  TOKEN_RPAREN,
  TOKEN_LARROW,
  TOKEN_PIPE,
  TOKEN_DOUBLE_SEMI,
  TOKEN_DBL_PERCENT,
  TOKEN_RARROW,
  TOKEN_DARROW,
  TOKEN_UARROW
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
  std::pair<TL::u32string, int>,
  TL::Tree::UnaryOperator,
  TL::Tree::BinaryOperator
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

	void dimid(const std::pair<TL::u32string, TL::u32string>& d)
	{
    INFO("Testing identifier: " << d.second);
    REQUIRE(m_current != m_tokens.end());

    const std::pair<TL::u32string, TL::u32string>* wsp 
      = TL::get<std::pair<TL::u32string, TL::u32string>>(&*m_current);

    //if this fails the type of the token is wrong
    REQUIRE(wsp != 0);
    CHECK(d.first == U"dim");
    CHECK(d.second == wsp->second);

    ++m_current;
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
  binop(const TL::Tree::BinaryOperator& op)
  {
    INFO("Testing binop: " << op.symbol);
    REQUIRE(m_current != m_tokens.end());

    auto opval = TL::get<TL::Tree::BinaryOperator>(&*m_current);

    REQUIRE(opval != nullptr);
    CHECK(opval->op == op.op);
    CHECK(opval->symbol == op.symbol);
    CHECK(opval->cbn == op.cbn);
    CHECK(opval->assoc == op.assoc);
    CHECK(opval->precedence == op.precedence);

    ++m_current;
  }
  
  void
  unaryop(const TL::Tree::UnaryOperator& op)
  {
    INFO("Testing unaryop: " << op.symbol);
    REQUIRE(m_current != m_tokens.end());

    auto opval = TL::get<TL::Tree::UnaryOperator>(&*m_current);

    REQUIRE(opval != nullptr);
    CHECK(opval->op == op.op);
    CHECK(opval->symbol == op.symbol);
    CHECK(opval->call_by_name == op.call_by_name);

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
        case TL::Parser::TOKEN_EOF:
        checker.symbol(TOKEN_EOF);
        break;

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

        case TL::Parser::TOKEN_UARROW:
        checker.symbol(TOKEN_UARROW);
        break;

        case TL::Parser::TOKEN_DARROW:
        checker.symbol(TOKEN_DARROW);
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
        case TL::Parser::TOKEN_FI:
        checker.keyword(KEYWORD_FI);
        break;

        case TL::Parser::TOKEN_IF:
        checker.keyword(KEYWORD_IF);
        break;

        case TL::Parser::TOKEN_THEN:
        checker.keyword(KEYWORD_THEN);
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
        case TL::Parser::TOKEN_CONSTANT:
        checker.constant(TL::get<std::pair<TL::u32string, TL::u32string>>
          (tok.getValue()));
        break;

        case TL::Parser::TOKEN_UCHAR:
        checker.character(TL::get<char32_t>(tok.getValue()));
        break;

        case TL::Parser::TOKEN_DIM_IDENTIFIER:
        checker.dimid(std::make_pair(U"dim", 
          TL::get<TL::u32string>(tok.getValue())));
        break;

        //identifier
        case TL::Parser::TOKEN_ID:
        checker.identifier(TL::get<TL::u32string>(tok.getValue()));
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
        case TL::Parser::TOKEN_POSTFIX_OP:
        checker.unaryop(TL::get<TL::Tree::UnaryOperator>(tok.getValue()));
        break;

        case TL::Parser::TOKEN_BINARY_OP:
        checker.binop(TL::get<TL::Tree::BinaryOperator>(tok.getValue()));
        break;

        //where
        case TL::Parser::TOKEN_WHERE:
        checker.where(TL::get<TL::u32string>(tok.getValue()));
        break;

        case TL::Parser::TOKEN_OPERATOR:
        INFO("Got TOKEN_OPERATOR");
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

  TL::Parser::U32Iterator baseBegin(
    TL::Parser::makeUTF32Iterator(input.begin()));

  TL::Parser::U32Iterator baseEnd(
    TL::Parser::makeUTF32Iterator(input.end())
  );

  TL::Parser::StreamPosIterator first
  (
    baseBegin,
    input
  );
  TL::Parser::StreamPosIterator last(baseEnd);

  return parse(first, last, k, idents, checker)
    && first == last;
}

bool check_utf8(const std::string& input, Checker& checker)
{
  TL::System& system = getSystem();
  TL::Context& k = system.getDefaultContext();
  TL::System::IdentifierLookup idents = system.lookupIdentifiers();

  TL::Parser::U32Iterator baseBegin(
    TL::Parser::makeUTF8Iterator(input.begin()));

  TL::Parser::U32Iterator baseEnd(
    TL::Parser::makeUTF8Iterator(input.end())
  );

  TL::Parser::StreamPosIterator first
  (
    baseBegin,
    TL::utf8_to_utf32(input)
  );
  TL::Parser::StreamPosIterator last(baseEnd);

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
    std::make_pair(U"uchar", U"a"),
    std::make_pair(U"uchar", U"\u0041"),
    std::make_pair(U"uchar", U"\u0041"),
    std::make_pair(U"uchar", U"\u00a2"),
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

TEST_CASE ( "symbols", "check all the symbols" )
{
  TL::u32string input = UR"*(: [ ] . = # @ \ \\ ( ) -> | ;; \\\ %% <- ↑ ↓)*";
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
    //std::make_pair(U"..", TL::Parser::TOKEN_BINARY_OP),
    TOKEN_LPAREN,
    TOKEN_RPAREN,
    TOKEN_RARROW,
    TOKEN_PIPE,
    TOKEN_DOUBLE_SEMI,
    TOKEN_DOUBLE_SLASH,
    TOKEN_SLASH,
    TOKEN_DBL_PERCENT,
    TOKEN_LARROW,
    TOKEN_UARROW,
    TOKEN_DARROW
  });

  check(input, checker);
}

TEST_CASE ( "mixed", "random test" )
{
  TL::u32string input = U"intmp @ 10 (hello if) cats [1 <- 5] arg0 arg1";
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
    TOKEN_RSQUARE,
    std::make_pair(U"dim", U"arg0"),
    std::make_pair(U"dim", U"arg1")
  });

  check(input, checker);
}

#if 0
TEST_CASE ( "operators", "check arbitrary operators" )
{
  INFO("testing the operator symbol");
  TL::u32string input = U"4 % 5 - + ..";
  Checker checker({
    mpz_class(4),
    TL::Tree::BinaryOperator(TL::Tree::ASSOC_LEFT, U"modulus", U"%", 10),
    mpz_class(5),
    TL::Tree::UnaryOperator(U"minus", U"-", TL::Tree::UNARY_PREFIX),
    TL::Tree::UnaryOperator(U"plus", U"+", TL::Tree::UNARY_POSTFIX),
    TL::Tree::BinaryOperator(TL::Tree::ASSOC_LEFT, U"range_construct", 
      U"..", 0)
  });

  check(input, checker);
}
#endif

TEST_CASE ( "utf8", "utf8 input stream" )
{
  INFO("testing utf8 input stream");
  std::string input = u8"%% 45 4 600 ξα";
  Checker checker({
    TOKEN_DBL_PERCENT,
    mpz_class(45),
    mpz_class(4),
    mpz_class(600),
    TL::u32string(U"ξα")
  });
  CHECK(check_utf8(input, checker));
}

TEST_CASE ( "comments", "check that comments work" )
{
  INFO("testing comments");

  std::string input = 
  R"(
    //comment
    y = 6;; //more comments
    z = 5;;
    //end comment
  )";

  Checker checker({
    U"y",
    TOKEN_EQUALS,
    mpz_class(6),
    TOKEN_DOUBLE_SEMI,
    U"z",
    TOKEN_EQUALS,
    mpz_class(5),
    TOKEN_DOUBLE_SEMI,
    TOKEN_EOF
  });

  check_utf8(input, checker);
}

TEST_CASE( "lexer iterator", "the lexer token stream iterator" )
{
  TL::System s;

  std::string input("42");

  TL::Parser::StreamPosIterator rawbegin
  (
    TL::Parser::U32Iterator(
      TL::Parser::makeUTF8Iterator(input.begin())
    ),
    U"42"
  );

  TL::Parser::StreamPosIterator rawend{
    TL::Parser::makeUTF8Iterator(input.end())
  };

  TL::Parser::LexerIterator begin
  (
    rawbegin,
    rawend,
    s.getDefaultContext(),
    s.lookupIdentifiers()
  );

  TL::Parser::LexerIterator other = begin;

  TL::Parser::LexerIterator end = begin.makeEnd();

  CHECK(begin->getType() == TL::Parser::TOKEN_INTEGER);
  ++begin;
  CHECK(*begin == 0);

  CHECK(begin == end);

  ++other;
  CHECK(other == end);
}

TEST_CASE( "expr parser", "basic expression parser tests" )
{
  TL::System s;

  TL::Parser::Parser p(s);

  std::string input("42");

  TL::Parser::StreamPosIterator rawbegin
  (
    TL::Parser::U32Iterator(
      TL::Parser::makeUTF8Iterator(input.begin())
    ),
    U"42"
  );

  TL::Parser::StreamPosIterator rawend{
    TL::Parser::makeUTF8Iterator(input.end())
  };

  TL::Parser::LexerIterator begin
  (
    rawbegin,
    rawend,
    s.getDefaultContext(),
    s.lookupIdentifiers()
  );

  TL::Parser::LexerIterator end = begin.makeEnd();

  TL::Tree::Expr result;
  bool success = p.parse_expr(begin, end, result);

  CHECK(success);
  CHECK(begin == end);
  CHECK(TL::get<mpz_class>(result) == 42);

  //function application
  std::string input2("f.d A B");

  TL::Parser::StreamPosIterator rawbegin2
  (
    TL::Parser::U32Iterator(
      TL::Parser::makeUTF8Iterator(input2.begin())
    ),
    U"f.d A B"
  );

  TL::Parser::StreamPosIterator rawend2(
    TL::Parser::makeUTF8Iterator(input2.end())
  );

  TL::Parser::LexerIterator begin2
  {
    rawbegin2,
    rawend2,
    s.getDefaultContext(),
    s.lookupIdentifiers()
  };
  TL::Parser::LexerIterator end2 = begin2.makeEnd();

  CHECK(p.parse_expr(begin2, end2, result));
  CHECK(TL::Printer::print_expr_tree(result) == "f.d A B");

  TL::Tree::PhiAppExpr* appB = TL::get<TL::Tree::PhiAppExpr>(&result);
  REQUIRE(appB != 0);

  TL::Tree::PhiAppExpr* appA = TL::get<TL::Tree::PhiAppExpr>(&appB->lhs);
  REQUIRE(appA != 0);

  TL::Tree::LambdaAppExpr* appd = TL::get<TL::Tree::LambdaAppExpr>
    (&appA->lhs);
  REQUIRE(appd != 0);

  s.addBinaryOperator(TL::Tree::BinaryOperator
    (
      TL::Tree::ASSOC_LEFT, U"plus", U"+", 200
    )
  );

  s.addBinaryOperator(TL::Tree::BinaryOperator
    (
      TL::Tree::ASSOC_LEFT, U"times", U"*", 400
    )
  );

  s.addBinaryOperator(TL::Tree::BinaryOperator
    (
      TL::Tree::ASSOC_LEFT, U"range_construct", U"..", 0
    )
  );

  #if 0

  std::string input3("5 + 6 + 7");

  TL::Parser::StreamPosIterator rawbegin3
  (
    TL::Parser::U32Iterator(
      TL::Parser::makeUTF8Iterator(input3.begin())
    ),
    U"5 + 6 + 7"
  );

  TL::Parser::StreamPosIterator rawend3
  (
    TL::Parser::makeUTF8Iterator(input3.end())
  );

  TL::Parser::LexerIterator begin3
  {
    rawbegin3,
    rawend3,
    s.getDefaultContext(),
    s.lookupIdentifiers()
  };
  TL::Parser::LexerIterator end3 = begin3.makeEnd();
  CHECK(p.parse_expr(begin3, end3, result));

  CHECK(TL::Printer::print_expr_tree(result) == "5 + 6 + 7");


  std::string input4("5 * (6 + 7)");

  TL::Parser::StreamPosIterator rawbegin4
  (
    TL::Parser::U32Iterator(
      TL::Parser::makeUTF8Iterator(input4.begin())
    ),
    U"5 * (6 + 7)"
  );

  TL::Parser::StreamPosIterator rawend4
  (
    TL::Parser::makeUTF8Iterator(input4.end())
  );

  TL::Parser::LexerIterator begin4
  {
    rawbegin4,
    rawend4,
    s.getDefaultContext(),
    s.lookupIdentifiers()
  };
  TL::Parser::LexerIterator end4 = begin4.makeEnd();
  CHECK(p.parse_expr(begin4, end4, result));

  CHECK(TL::Printer::print_expr_tree(result) == "5 * (6 + 7)");


  std::string input5("1 .. 5");

  TL::Parser::StreamPosIterator rawbegin5
  (
    TL::Parser::U32Iterator(
      TL::Parser::makeUTF8Iterator(input5.begin())
    ),
    U"1 .. 5"
  );

  TL::Parser::StreamPosIterator rawend5
  (
    TL::Parser::makeUTF8Iterator(input5.end())
  );

  TL::Parser::LexerIterator begin5
  {
    rawbegin5,
    rawend5,
    s.getDefaultContext(),
    s.lookupIdentifiers()
  };
  TL::Parser::LexerIterator end5 = begin5.makeEnd();
  CHECK(p.parse_expr(begin5, end5, result));

  CHECK(TL::Printer::print_expr_tree(result) == "1 .. 5");
  #endif
}
