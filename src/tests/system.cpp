/* System tests.
   Copyright (C) 2009, 2010, 2011 Jarryd Beck and John Plaice

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

/**
 * @file system.cpp
 * System tests.
 */

#include <gmpxx.h>

#include <tl/context.hpp>
#include <tl/line_tokenizer.hpp>
#include <tl/output.hpp>
#include <tl/parser_iterator.hpp>
#include <tl/types.hpp>
#include <tl/types/intmp.hpp>

#include <boost/spirit/include/classic_multi_pass.hpp>
#include <boost/spirit/include/support_istream_iterator.hpp>

#define CATCH_CONFIG_MAIN
#include "catch.hpp"

namespace TL = TransLucid;

//first some iterator tests

TEST_CASE ( "utf8 iterator comparison", 
  "two constructed iterators compare equal" )
{
  std::string s = "test";
  TL::Parser::U32Iterator iter3(TL::Parser::makeUTF8Iterator(s.begin()));

  TL::Parser::U32Iterator iter4(TL::Parser::makeUTF8Iterator(s.begin()));

  CHECK(iter3 == iter4);
}

TEST_CASE ( "utf8 iterator end", "go to the end, do they compare equal" )
{
  std::string s = "ab";

  auto utf8iter = TL::Parser::makeUTF8Iterator(s.begin());
  auto utf8iter_end = TL::Parser::makeUTF8Iterator(s.end());

  ++utf8iter;
  ++utf8iter;
  CHECK(utf8iter == utf8iter_end);

  TL::Parser::U32Iterator iter(TL::Parser::makeUTF8Iterator(s.begin()));
  TL::Parser::U32Iterator end(TL::Parser::makeUTF8Iterator(s.end()));

  CHECK(iter != end);

  ++iter;
  ++iter;
  CHECK(iter == end);

}

TEST_CASE ( "utf8 iterator copying", "do the iterators copy correctly" )
{
  std::string s = "test";
  TL::Parser::U32Iterator iter(TL::Parser::makeUTF8Iterator(s.begin()));

  TL::Parser::U32Iterator end(
    TL::Parser::makeUTF8Iterator(s.end())
  );

  TL::u32string copy;

  while (iter != end)
  {
    unsigned int c = *iter;
    copy += c;
    ++iter;
  }

  TL::u32string correct = U"test";
  CHECK(copy == correct);
}

TEST_CASE ( "utf8 iterator copy increment", "copy then increment" )
{
  std::string s = "hello world";

  TL::Parser::U32Iterator iter(TL::Parser::makeUTF8Iterator(s.begin()));

  auto iter2 = iter;

  CHECK(*iter == U'h');

  ++iter;

  CHECK(*iter == U'e');
  CHECK(*iter2 == U'h');
}

TEST_CASE ( "utf8 iterator non-ascii", 
  "does the iterator read a non-ascii character correctly")
{
  std::string s = u8"\u00e9";
  TL::Parser::U32Iterator iter(TL::Parser::makeUTF8Iterator(s.begin()));

  unsigned int c = *iter;

  CHECK(c == U'\u00e9');
}

//not doing streams anymore
#if 0
TEST_CASE ( "utf8 iterator stream", 
  "does the iterator read from a stream correctly" )
{
  std::istringstream is("%%\n%%\n5;;");
  is >> std::noskipws;

  TL::Parser::U32Iterator pos
  (
    TL::Parser::makeUTF8Iterator
    (
      boost::spirit::istream_iterator(is)
    ),
    TL::Parser::makeUTF8Iterator(boost::spirit::istream_iterator())
  );

  TL::Parser::U32Iterator end;

  TL::u32string s;
  while (pos != end)
  {
    s += *pos;
    ++pos;
  }

  CHECK(s == TL::u32string(U"%%\n%%\n5;;"));
}

TEST_CASE( "utf8 iterator stream increment", 
  "read from a stream and increment" )
{
  std::istringstream is("1234567890");
  is >> std::noskipws;

  boost::spirit::istream_iterator iter(is);

  TL::Parser::U32Iterator pos_original
  (
    TL::Parser::makeUTF8Iterator
    (
      iter
    ),
    TL::Parser::makeUTF8Iterator(boost::spirit::istream_iterator())
  );

  TL::Parser::U32Iterator pos1 = pos_original;
  TL::Parser::U32Iterator pos = pos1;
  TL::Parser::U32Iterator pos2 = pos1;
  TL::Parser::U32Iterator end;

  CHECK(pos != end);

  CHECK(*pos++ == U'1');
  pos2 = pos;
  CHECK(pos != end);

  CHECK(*pos++ == U'2');
  pos2 = pos;
  CHECK(pos != end);

  CHECK(*pos++ == U'3');
  pos2 = pos;
  CHECK(pos != end);

  CHECK(*pos++ == U'4');
  pos2 = pos;
  CHECK(pos != end);

  CHECK(*pos++ == U'5');
  pos2 = pos;
  CHECK(pos != end);

  CHECK(*pos++ == U'6');
  pos2 = pos;
  CHECK(pos != end);
}

TEST_CASE( "utf8 iterator stream weird", 
  "some convoluted iterator stream test" )
{
  std::istringstream is("%%%%5;;");
  is >> std::noskipws;

  boost::spirit::istream_iterator iter(is);

  TL::Parser::U32Iterator pos
  (
    TL::Parser::makeUTF8Iterator
    (
      iter
    ),
    TL::Parser::makeUTF8Iterator(boost::spirit::istream_iterator())
  );

  CHECK(*pos++ == U'%');
  CHECK(*pos++ == U'%');
  CHECK(*pos++ == U'%');
  CHECK(*pos++ == U'%');
  CHECK(*pos++ == U'5');
  CHECK(*pos++ == U';');
  CHECK(*pos++ == U';');
  CHECK(pos == TL::Parser::U32Iterator());
}

TEST_CASE( "utf8 iterator multi_pass", "iterator with multi_pass" )
{
  std::istringstream is("abcdefghijklmnop");
  boost::spirit::classic::multi_pass<TL::Parser::U32Iterator> pos
  (
    TL::Parser::U32Iterator
    (
      TL::Parser::makeUTF8Iterator(boost::spirit::istream_iterator(is)),
      TL::Parser::makeUTF8Iterator(boost::spirit::istream_iterator())
    )
  );

  auto pos2 = pos;
  auto segment_begin = pos;
  CHECK(*pos == U'a');

  for (int i = 0; i != 7; ++i)
  {
    ++pos;
  }

  CHECK(*pos == U'h');
  auto segment_end = pos;

  TL::u32string result;
  for (int i = 0; i != 16; ++i)
  {
    result += *pos2;
    ++pos2;
  }

  CHECK(result == TL::u32string(U"abcdefghijklmnop"));

  CHECK(pos2 == 
    boost::spirit::classic::multi_pass<TL::Parser::U32Iterator>());

  result.clear();
  for (int i = 0; i != 8; ++i)
  {
    result += *pos;
    ++pos;
  }

  result += *pos;
  CHECK(pos !=
    boost::spirit::classic::multi_pass<TL::Parser::U32Iterator>());

  CHECK(result == TL::u32string(U"hijklmnop"));

  ++pos;
  CHECK(pos == 
    boost::spirit::classic::multi_pass<TL::Parser::U32Iterator>());

  CHECK(TL::u32string(segment_begin, segment_end) ==
    TL::u32string(U"abcdefg"));
}
#endif

#if 0
TEST_CASE ( comparison )
{
  TL::u32string s(U"test");
  TL::Parser::U32Iterator iter1(TL::Parser::makeUTF32Iterator(s.begin()));
  TL::Parser::U32Iterator iter2(TL::Parser::makeUTF32Iterator(s.begin()));
  CHECK(iter1 == iter2);

  TL::Parser::U32Iterator iter3(TL::Parser::makeUTF32Iterator(s.end()));
  TL::Parser::U32Iterator iter4(TL::Parser::makeUTF32Iterator(s.end()));
  CHECK(iter3 == iter4);
}
#endif

TEST_CASE ( "utf32 iterator end", "the end iterator" )
{
  TL::u32string s = U"ab";
  TL::Parser::U32Iterator iter(TL::Parser::makeUTF32Iterator(s.begin()));
  TL::Parser::U32Iterator end(TL::Parser::makeUTF32Iterator(s.end()));

  CHECK(iter != end);

  ++iter;
  ++iter;
  CHECK(iter == end);
}

TEST_CASE ( "utf32 iterator copying", "can we copy them" )
{
  TL::u32string s(U"test");
  TL::u32string result;

  TL::Parser::U32Iterator iter(TL::Parser::makeUTF32Iterator(s.begin()));
  TL::Parser::U32Iterator end(TL::Parser::makeUTF32Iterator(s.end()));

  while (iter != end)
  {
    result += *iter;
    ++iter;
  }

  CHECK(s == result);
}

//need to test this since lex uses multipass
//no more multipass
#if 0
TEST_CASE( "utf32 iterator multi_pass", "can we use it with multi_pass" )
{
  TL::u32string s = U"abcdefghijklmnop";
  boost::spirit::classic::multi_pass<TL::Parser::U32Iterator> pos
  (
    TL::Parser::U32Iterator
    (
      TL::Parser::makeUTF32Iterator(s.begin()),
      TL::Parser::makeUTF32Iterator(s.end())
    )
  );

  auto pos2 = pos;
  CHECK(*pos == U'a');

  for (int i = 0; i != 7; ++i)
  {
    ++pos;
  }

  CHECK(*pos == U'h');

  TL::u32string result;
  for (int i = 0; i != 16; ++i)
  {
    result += *pos2;
    ++pos2;
  }

  CHECK(result == TL::u32string(U"abcdefghijklmnop"));

  CHECK(pos2 == 
    boost::spirit::classic::multi_pass<TL::Parser::U32Iterator>());

  result.clear();
  for (int i = 0; i != 9; ++i)
  {
    result += *pos;
    ++pos;
  }
  CHECK(result == TL::u32string(U"hijklmnop"));
  CHECK(pos == 
    boost::spirit::classic::multi_pass<TL::Parser::U32Iterator>());
}
#endif

TEST_CASE( "utf32 iterator copy", "copy the iterator")
{ 
  TL::u32string s = U"test";
  TL::Parser::U32Iterator iter
  (
    TL::Parser::makeUTF32Iterator(s.begin())
  );
  TL::Parser::U32Iterator end(
    TL::Parser::makeUTF32Iterator(s.end())
  );

  TL::Parser::U32Iterator savePos = iter;

  CHECK(*iter == U't');

  ++iter;
  CHECK(*iter == U'e');

  TL::Parser::U32Iterator iter2 = iter;
  CHECK(*iter2 == U'e');

  ++iter;
  CHECK(*iter == U's');
  CHECK(*iter2 == U'e');

  ++iter2;
  CHECK(*iter2 == U's');

  TL::Parser::U32Iterator fakeEnd = iter;

  ++iter2;
  CHECK(*iter2 == U't');

  ++iter;
  CHECK(iter == iter2);

  ++iter;
  ++iter2;
  CHECK(iter == iter2);
  CHECK(iter == end);
  CHECK(iter2 == end);

  CHECK(TL::u32string(savePos, end) == U"test");
  CHECK(TL::u32string(savePos, fakeEnd) == TL::u32string(U"te"));
}

//now some line tokenizer tests

TEST_CASE( "line tokenizer empty input", "is it correct with empty input")
{
  std::string input;

  TL::Parser::U32Iterator iter(
    TL::Parser::makeUTF8Iterator(input.begin()));
  TL::Parser::U32Iterator end(
    TL::Parser::makeUTF8Iterator(input.end())
  );

  TL::LineTokenizer tokenize(iter, end);

  auto n = tokenize.next();

  CHECK(n.first == TL::LineType::EMPTY);
  CHECK(n.second == TL::u32string());
}

TEST_CASE( "line tokenizer simple splitting", "some simply tests" )
{
  std::string input = "eqn a = 5;; assign y := 6;;";

  TL::Parser::U32Iterator iter(
    TL::Parser::makeUTF8Iterator(input.begin()));
  TL::Parser::U32Iterator end(
    TL::Parser::makeUTF8Iterator(input.end())
  );

  TL::LineTokenizer tokenize(iter, end);

  auto n = tokenize.next();
  CHECK(n.first == TL::LineType::LINE);
  CHECK(n.second == TL::u32string(U"eqn a = 5;;"));

  n = tokenize.next();
  CHECK(n.first == TL::LineType::LINE);
  CHECK(n.second == TL::u32string(U"assign y := 6;;"));
}

TEST_CASE( "line tokenizer $ symbol", "does it handle $$ correctly" )
{
  std::string input = "eqn a = b;;  $$";

  TL::Parser::U32Iterator iter(
    TL::Parser::makeUTF8Iterator(input.begin()));
  TL::Parser::U32Iterator end(
    TL::Parser::makeUTF8Iterator(input.end())
  );

  TL::LineTokenizer tokenize(iter, end);

  auto n = tokenize.next();

  CHECK(n.first == TL::LineType::LINE);
  CHECK(n.second == TL::u32string(U"eqn a = b;;"));

  n = tokenize.next();
  CHECK(n.first == TL::LineType::DOUBLE_DOLLAR);
  CHECK(n.second == TL::u32string(U"$$"));

  n = tokenize.next();
  CHECK(n.first == TL::LineType::EMPTY);
  CHECK(n.second == TL::u32string());
}

TEST_CASE( "line tokenizer extra spaces", "arbitrary white space to skip" )
{
  std::string input = "eqn a = b;;  $$  ";

  TL::Parser::U32Iterator iter(
    TL::Parser::makeUTF8Iterator(input.begin()));
  TL::Parser::U32Iterator end(
    TL::Parser::makeUTF8Iterator(input.end())
  );

  TL::LineTokenizer tokenize(iter, end);

  auto n = tokenize.next();
  CHECK(n.first == TL::LineType::LINE);
  CHECK(n.second == TL::u32string(U"eqn a = b;;"));

  n = tokenize.next();
  CHECK(n.first == TL::LineType::DOUBLE_DOLLAR);
  CHECK(n.second == TL::u32string(U"$$"));

  n = tokenize.next();
  CHECK(n.first == TL::LineType::EMPTY);
  CHECK(n.second == TL::u32string());
}

TEST_CASE( "line tokenizer %% symbol", "does it handle %% correctly"  )
{
  std::string input = "eqn x = 42;;\n%%\nx;;";

  TL::Parser::U32Iterator iter(
    TL::Parser::makeUTF8Iterator(input.begin()));
  TL::Parser::U32Iterator end(
    TL::Parser::makeUTF8Iterator(input.end())
  );

  TL::LineTokenizer tokenize(iter, end);

  auto n = tokenize.next();
  CHECK(n.first == TL::LineType::LINE);
  CHECK(n.second == TL::u32string(U"eqn x = 42;;"));

  n = tokenize.next();
  CHECK(n.first == TL::LineType::DOUBLE_PERCENT);
  CHECK(n.second == TL::u32string(U"%%"));

  n = tokenize.next();
  CHECK(n.first == TL::LineType::LINE);
  CHECK(n.second == TL::u32string(U"x;;"));

  n = tokenize.next();
  CHECK(n.first == TL::LineType::EMPTY);
  CHECK(n.second == TL::u32string());
}

TEST_CASE( "context manipulation", "perturb and restore some contexts")
{
  TL::Context k;

  TL::Constant v1 = TL::Types::Intmp::create(5);
  TL::Constant v2 = TL::Types::Intmp::create(6);
  TL::Constant v3 = TL::Types::Intmp::create(7);
  TL::Constant v4 = TL::Types::Intmp::create(8);

  TL::Tuple t1(TL::tuple_t{
    {0, v1},
    {1, v2}
  });

  k.perturb(t1);

  CHECK(k.lookup(0) == v1);
  CHECK(k.lookup(1) == v2);

  TL::Tuple t2(TL::tuple_t{
    {0, v3},
    {2, v4}
  });

  k.perturb(t2);

  CHECK(k.lookup(0) == v3);
  CHECK(k.lookup(1) == v2);
  CHECK(k.lookup(2) == v4);

  k.restore(t2);

  CHECK(k.lookup(0) == v1);
  CHECK(k.lookup(1) == v2);

  TL::Tuple t3(TL::tuple_t{
    {-6, v1},
    {-2, v2}
  });

  k.perturb(t3);
  CHECK(k.lookup(-6) == v1);
  CHECK(k.lookup(-2) == v2);
}
