/* Core TransLucid application implementation.
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

#include <tl/expr_parser.hpp>
#include <tl/equation_parser.hpp>
#include <tl/line_tokenizer.hpp>
#include <tl/tuple_parser.hpp>
#include <tl/system.hpp>
#include <tl/expr_compiler.hpp>

#include <iterator>
#include <iostream>

#include <boost/spirit/include/support_istream_iterator.hpp>

#include "tltext.hpp"

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

/**
 * @file tlcore.cpp
 * The tlcore application. All of the code which runs the main tlcore
 * application.
 */

namespace TransLucid
{

namespace TLText
{

struct Instant
{
  void
  operator()(const Parser::Instant& i)
  {
  }
};

TLText::TLText()
: 
  m_verbose(false)
 ,m_reactive(false)
 ,m_demands(false)
 ,m_uuids(false)
 ,m_is(&std::cin)
 ,m_os(&std::cout)
 ,m_compiler(&m_system)
 ,m_time(0)
 ,m_lastLibLoaded(0)
{
  m_libtool.addSearchPath(to_u32string(std::string(PREFIX "/share/tl")));
}

void 
TLText::run()
{
  std::cerr << "TLText..." << std::endl;
  *m_is >> std::noskipws;

  Parser::U32Iterator begin(
    Parser::makeUTF8Iterator(boost::spirit::istream_iterator(*m_is)),
    Parser::makeUTF8Iterator(boost::spirit::istream_iterator())
  );

  Parser::U32Iterator end;

  #if 0
  Constant c;
  while (begin != end)
  {
    c = m_system.parseLine(begin, end);
    std::cerr << "got Constant of type " << c.index() << std::endl;
  }
  #endif

  LineTokenizer tokenizer(begin);
  while (!tokenizer.end())
  {
    auto line = tokenizer.next();

    Parser::U32Iterator lineBegin(
      Parser::makeUTF32Iterator(line.second.begin()),
      Parser::makeUTF32Iterator(line.second.end())
    );

    m_system.parseLine(lineBegin);
  }
}

} //namespace TLCore

} //namespace TransLucid
