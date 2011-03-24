/* Iterator tests.
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

/**
 * @file iterator.cpp
 * Iterator tests.
 */

#include <tl/parser_iterator.hpp>
#include <tl/types.hpp>
//#include <tl/parser_fwd.hpp>

#include <boost/spirit/include/classic_multi_pass.hpp>

#define BOOST_TEST_MODULE U32Iterator
#include <boost/test/included/unit_test.hpp>

namespace TL = TransLucid;

BOOST_AUTO_TEST_SUITE( utf8_iterator_tests )

BOOST_AUTO_TEST_CASE ( comparison )
{
  std::string s = "test";
  TL::Parser::U32Iterator iter3(TL::Parser::makeUTF8Iterator(s.begin()),
    TL::Parser::makeUTF8Iterator(s.end()));
  TL::Parser::U32Iterator iter4(TL::Parser::makeUTF8Iterator(s.begin()),
    TL::Parser::makeUTF8Iterator(s.end()));
  BOOST_CHECK(iter3 == iter4);
}

BOOST_AUTO_TEST_CASE ( end )
{
  std::string s = "ab";

  auto utf8iter = TL::Parser::makeUTF8Iterator(s.begin());
  auto utf8iter_end = TL::Parser::makeUTF8Iterator(s.end());

  ++utf8iter;
  ++utf8iter;
  BOOST_CHECK(utf8iter == utf8iter_end);

  TL::Parser::U32Iterator iter(TL::Parser::makeUTF8Iterator(s.begin()),
    TL::Parser::makeUTF8Iterator(s.end()));

  BOOST_CHECK(iter != TL::Parser::U32Iterator());

  ++iter;
  ++iter;
  BOOST_CHECK(iter == TL::Parser::U32Iterator());

}

BOOST_AUTO_TEST_CASE ( copying )
{
  std::string s = "test";
  TL::Parser::U32Iterator iter(TL::Parser::makeUTF8Iterator(s.begin()),
    TL::Parser::makeUTF8Iterator(s.end()));

  std::basic_string<unsigned int> copy;

  while (iter != TL::Parser::U32Iterator())
  {
    unsigned int c = *iter;
    copy += c;
    ++iter;
  }

  std::basic_string<unsigned int> correct = {'t', 'e', 's', 't'};
  BOOST_CHECK(copy == correct);
}

BOOST_AUTO_TEST_CASE ( nonascii )
{
  std::string s = u8"\u00e9";
  TL::Parser::U32Iterator iter(TL::Parser::makeUTF8Iterator(s.begin()),
    TL::Parser::makeUTF8Iterator(s.end()));

  unsigned int c = *iter;

  BOOST_CHECK_EQUAL(c, U'\u00e9');
}

BOOST_AUTO_TEST_CASE ( stream )
{
  std::istringstream is("%%\n%%\n5;;");
  is >> std::noskipws;

  TL::Parser::U32Iterator pos
  (
    TL::Parser::makeUTF8Iterator
    (
      std::istream_iterator<char>(is)
    ),
    TL::Parser::makeUTF8Iterator(std::istream_iterator<char>())
  );

  TL::Parser::U32Iterator end;

  TL::u32string s;
  while (pos != end)
  {
    s += *pos;
    ++pos;
  }

  BOOST_CHECK_EQUAL(s, TL::u32string(U"%%\n%%\n5;;"));
}

BOOST_AUTO_TEST_CASE( stream_increment )
{
  std::istringstream is("1234567890");
  is >> std::noskipws;

  std::istream_iterator<char> iter(is);

  TL::Parser::U32Iterator pos_original
  (
    TL::Parser::makeUTF8Iterator
    (
      iter
    ),
    TL::Parser::makeUTF8Iterator(std::istream_iterator<char>())
  );

  TL::Parser::U32Iterator pos1 = pos_original;
  TL::Parser::U32Iterator pos = pos1;
  TL::Parser::U32Iterator pos2 = pos1;
  TL::Parser::U32Iterator end;

  BOOST_CHECK(pos != end);

  BOOST_CHECK_EQUAL(*pos++, '1');
  pos2 = pos;
  BOOST_CHECK(pos != end);

  BOOST_CHECK_EQUAL(*pos++, '2');
  pos2 = pos;
  BOOST_CHECK(pos != end);

  BOOST_CHECK_EQUAL(*pos++, '3');
  pos2 = pos;
  BOOST_CHECK(pos != end);

  BOOST_CHECK_EQUAL(*pos++, '4');
  pos2 = pos;
  BOOST_CHECK(pos != end);

  BOOST_CHECK_EQUAL(*pos++, '5');
  pos2 = pos;
  BOOST_CHECK(pos != end);

  BOOST_CHECK_EQUAL(*pos++, '6');
  pos2 = pos;
  BOOST_CHECK(pos != end);
}

BOOST_AUTO_TEST_CASE( stream_weird )
{
  std::istringstream is("%%%%5;;");
  is >> std::noskipws;

  std::istream_iterator<char> iter(is);

  TL::Parser::U32Iterator pos
  (
    TL::Parser::makeUTF8Iterator
    (
      iter
    ),
    TL::Parser::makeUTF8Iterator(std::istream_iterator<char>())
  );

  BOOST_CHECK_EQUAL(*pos++, '%');
  BOOST_CHECK_EQUAL(*pos++, '%');
  BOOST_CHECK_EQUAL(*pos++, '%');
  BOOST_CHECK_EQUAL(*pos++, '%');
  BOOST_CHECK_EQUAL(*pos++, '5');
  BOOST_CHECK_EQUAL(*pos++, ';');
  BOOST_CHECK_EQUAL(*pos++, ';');
  BOOST_CHECK(pos == TL::Parser::U32Iterator());
}

BOOST_AUTO_TEST_CASE( multi_pass )
{
  std::istringstream is("abcdefghijklmnop");
  boost::spirit::classic::multi_pass<TL::Parser::U32Iterator> pos
  (
    TL::Parser::U32Iterator
    (
      TL::Parser::makeUTF8Iterator(std::istream_iterator<char>(is)),
      TL::Parser::makeUTF8Iterator(std::istream_iterator<char>())
    )
  );

  auto pos2 = pos;
  auto segment_begin = pos;
  BOOST_CHECK(*pos == 'a');

  for (int i = 0; i != 7; ++i)
  {
    ++pos;
  }

  BOOST_CHECK(*pos == 'h');
  auto segment_end = pos;

  TL::u32string result;
  for (int i = 0; i != 16; ++i)
  {
    result += *pos2;
    ++pos2;
  }

  BOOST_CHECK_EQUAL(result, TL::u32string(U"abcdefghijklmnop"));

  BOOST_CHECK(pos2 == 
    boost::spirit::classic::multi_pass<TL::Parser::U32Iterator>());

  result.clear();
  for (int i = 0; i != 8; ++i)
  {
    result += *pos;
    ++pos;
  }

  result += *pos;
  BOOST_CHECK(pos !=
    boost::spirit::classic::multi_pass<TL::Parser::U32Iterator>());

  BOOST_CHECK_EQUAL(result, TL::u32string(U"hijklmnop"));

  ++pos;
  BOOST_CHECK(pos == 
    boost::spirit::classic::multi_pass<TL::Parser::U32Iterator>());

  BOOST_CHECK_EQUAL(TL::u32string(segment_begin, segment_end), 
    TL::u32string(U"abcdefg"));
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE( utf32_iterator_tests)
#if 0
BOOST_AUTO_TEST_CASE ( comparison )
{
  TL::u32string s(U"test");
  TL::Parser::U32Iterator iter1(TL::Parser::makeUTF32Iterator(s.begin()));
  TL::Parser::U32Iterator iter2(TL::Parser::makeUTF32Iterator(s.begin()));
  BOOST_CHECK(iter1 == iter2);

  TL::Parser::U32Iterator iter3(TL::Parser::makeUTF32Iterator(s.end()));
  TL::Parser::U32Iterator iter4(TL::Parser::makeUTF32Iterator(s.end()));
  BOOST_CHECK(iter3 == iter4);
}
#endif

BOOST_AUTO_TEST_CASE ( end )
{
  TL::u32string s = U"ab";
  TL::Parser::U32Iterator iter(TL::Parser::makeUTF32Iterator(s.begin()),
    TL::Parser::makeUTF32Iterator(s.end()));

  BOOST_CHECK(iter != TL::Parser::U32Iterator());

  ++iter;
  ++iter;
  BOOST_CHECK(iter == TL::Parser::U32Iterator());
}

BOOST_AUTO_TEST_CASE ( copying )
{
  TL::u32string s(U"test");
  TL::u32string result;

  TL::Parser::U32Iterator iter(TL::Parser::makeUTF32Iterator(s.begin()),
    TL::Parser::makeUTF32Iterator(s.end()));

  while (iter != TL::Parser::U32Iterator())
  {
    result += *iter;
    ++iter;
  }

  BOOST_CHECK(s == result);
}

//need to test this since lex uses multipass
BOOST_AUTO_TEST_CASE( multi_pass )
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
  BOOST_CHECK(*pos == 'a');

  for (int i = 0; i != 7; ++i)
  {
    ++pos;
  }

  BOOST_CHECK(*pos == 'h');

  TL::u32string result;
  for (int i = 0; i != 16; ++i)
  {
    result += *pos2;
    ++pos2;
  }

  BOOST_CHECK_EQUAL(result, TL::u32string(U"abcdefghijklmnop"));

  BOOST_CHECK(pos2 == 
    boost::spirit::classic::multi_pass<TL::Parser::U32Iterator>());

  result.clear();
  for (int i = 0; i != 9; ++i)
  {
    result += *pos;
    ++pos;
  }
  BOOST_CHECK_EQUAL(result, TL::u32string(U"hijklmnop"));
  BOOST_CHECK(pos == 
    boost::spirit::classic::multi_pass<TL::Parser::U32Iterator>());
}

BOOST_AUTO_TEST_CASE( copy_iter)
{ 
  TL::u32string s = U"test";
  TL::Parser::U32Iterator iter
  (
    TL::Parser::makeUTF32Iterator(s.begin()),
    TL::Parser::makeUTF32Iterator(s.end())
  );
  TL::Parser::U32Iterator end;

  TL::Parser::U32Iterator savePos = iter;

  BOOST_CHECK(*iter == 't');

  ++iter;
  BOOST_CHECK(*iter == 'e');

  TL::Parser::U32Iterator iter2 = iter;
  BOOST_CHECK(*iter2 == 'e');

  ++iter;
  BOOST_CHECK(*iter == 's');
  BOOST_CHECK(*iter2 == 'e');

  ++iter2;
  BOOST_CHECK(*iter2 == 's');

  TL::Parser::U32Iterator fakeEnd = iter;

  ++iter2;
  BOOST_CHECK(*iter2 == 't');

  ++iter;
  BOOST_CHECK(iter == iter2);

  ++iter;
  ++iter2;
  BOOST_CHECK(iter == iter2);
  BOOST_CHECK(iter == end);
  BOOST_CHECK(iter2 == end);

  BOOST_CHECK(TL::u32string(savePos, end) == U"test");
  BOOST_CHECK_EQUAL(TL::u32string(savePos, fakeEnd), TL::u32string(U"te"));
}

BOOST_AUTO_TEST_SUITE_END()
