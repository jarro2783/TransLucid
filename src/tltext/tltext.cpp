/* Core TransLucid application implementation.
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

#include <tl/expr_parser.hpp>
#include <tl/equation_parser.hpp>
#include <tl/hyperdatons/arrayhd.hpp>
#include <tl/line_tokenizer.hpp>
#include <tl/output.hpp>
#include <tl/tree_printer.hpp>
#include <tl/types/dimension.hpp>
#include <tl/types_util.hpp>
#include <tl/tuple_parser.hpp>
#include <tl/system.hpp>

#include <iterator>
#include <iostream>
#include <fstream>

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

TLText::TLText()
: 
  m_verbose(false)
 ,m_reactive(false)
 ,m_uuids(false)
 ,m_is(&std::cin)
 ,m_os(&std::cout)
 ,m_time(0)
 ,m_lastLibLoaded(0)
 ,m_argsHD(0)
{
  m_libtool.addSearchPath(to_u32string(std::string(PREFIX "/share/tl")));

  m_demands = new DemandHD(m_system);

  m_system.addOutputHyperdaton(U"demand", m_demands);

  m_headers.push_back(PREFIX "/share/tl/tltext/header.tl");
}

TLText::~TLText()
{
  delete m_demands;
  delete m_argsHD;
  delete m_envHD;
}

void
TLText::setup_hds()
{
  //command line arguments
  setup_clargs();
  //environment variables
  setup_envhd();
}

void 
TLText::run()
{
  setup_hds();

  //load up headers
  for (auto h : m_headers)
  {
    std::ifstream is(h.c_str());

    if (is)
    {
      is >> std::noskipws;
      Parser::U32Iterator begin(
        Parser::makeUTF8Iterator(boost::spirit::istream_iterator(is)),
        Parser::makeUTF8Iterator(boost::spirit::istream_iterator())
      );

      LineTokenizer tokens(begin);
      processDefinitions(tokens);
    }
  }

  std::cerr << "TLText..." << std::endl;
  *m_is >> std::noskipws;

  Parser::U32Iterator begin(
    Parser::makeUTF8Iterator(boost::spirit::istream_iterator(*m_is)),
    Parser::makeUTF8Iterator(boost::spirit::istream_iterator())
  );

  Parser::U32Iterator end;

  LineTokenizer tokenizer(begin);

  bool done = false;
  while (!done)
  {
    auto defs = processDefinitions(tokenizer);

    //only continue if the instant is valid
    if (defs.first)
    {
      std::vector<Tree::Expr> exprs;
      //only parse expressions if the right tokens were seen
      if (defs.second)
      {
        exprs = processExpressions(tokenizer);
      }

      //now process the instant

      //add equations for the exprs entered
      int slot = 0;
      size_t time = m_system.theTime();

      for (const Tree::Expr& e : exprs)
      {
        m_system.addAssignment(Parser::Equation
        (
          U"demand",
          Tree::TupleExpr
          (
            {
              {Tree::DimensionExpr(U"time"), mpz_class(time)},
              {Tree::DimensionExpr(U"slot"), mpz_class(slot)}
            }
          ),
          Tree::Expr(),

          Tree::AtExpr(
            Tree::IdentExpr(U"PRINT_TLTEXT"),
            Tree::TupleExpr({{Tree::DimensionExpr(U"arg0"), e}})
          )
        ));
        ++slot;
      }

      //run the demands
      m_system.go();

      //print some stuff
      for (int s = 0; s != slot; ++s)
      {
        const auto& c = (*m_demands)(s);
        if (c.index() == TYPE_INDEX_USTRING)
        {
          std::cout << get_constant_pointer<u32string>(c) << std::endl;
        }
        else
        {
          std::cerr << "Error: PRINT didn't return a string" << std::endl;
          std::cerr << "Type index: " << c.index() << std::endl;
        }

        #if 0
        if (c.index() == TYPE_INDEX_INTMP)
        {
          std::cout << get_constant_pointer<mpz_class>(c)
            << std::endl;
        }
        else if (c.index() == TYPE_INDEX_SPECIAL)
        {
          std::cout << "special: " << get_constant<Special>(c)
            << std::endl;
        }
        else if (c.index() == TYPE_INDEX_USTRING)
        {
          std::cout << "\"" << get_constant_pointer<u32string>(c) << "\""
            << std::endl;
        }
        else if (c.index() == TYPE_INDEX_UCHAR)
        {
          std::cout << "'" << u32string(1, get_constant<char32_t>(c)) << "'"
            << std::endl;
        }
        else if (c.index() == TYPE_INDEX_BOOL)
        {
          std::cout << std::boolalpha << get_constant<bool>(c) 
                    << std::noboolalpha << std::endl;
        }
        else
        {
          std::cerr << "index: " << c.index() << std::endl;
        }
        #endif
      }
    }
    else
    {
      done = true;
    }
  }
}

std::pair<bool, bool>
TLText::processDefinitions(LineTokenizer& tokenizer)
{
  bool instantValid = true;
  bool parseExpressions = true;
  bool first = true;

  bool done = false;
  while (!done)
  {
    auto line = tokenizer.next();
    switch(line.first)
    {
      case LineType::LINE:
      //parse a line with the system
      {
        Parser::U32Iterator lineBegin(
          Parser::makeUTF32Iterator(line.second.begin()),
          Parser::makeUTF32Iterator(line.second.end())
        );

        auto result = m_system.parseLine(lineBegin, m_verbose);
      }
      break;

      case LineType::DOUBLE_DOLLAR:
      parseExpressions = false;
      done = true;
      break;

      case LineType::EMPTY:
      if (first)
      {
        instantValid = false;
      }
      parseExpressions = false;
      done = true;
      break;

      case LineType::DOUBLE_PERCENT:
      done = true;
      break;
    }
    first = false;
  }

  return std::make_pair(instantValid, parseExpressions);
}

std::vector<Tree::Expr>
TLText::processExpressions(LineTokenizer& tokenizer)
{
  std::vector<Tree::Expr> exprs;
  bool done = false;
  while (!done)
  {
    auto line = tokenizer.next();

    switch(line.first)
    {
      case LineType::LINE:
      //parse an expression
      {
        Parser::U32Iterator lineBegin(
          Parser::makeUTF32Iterator(line.second.begin()),
          Parser::makeUTF32Iterator(line.second.end())
        );

        auto expr = m_system.parseExpression(lineBegin);
        if (expr.first)
        {
          if (m_verbose)
          {
            std::cout << print_expr_tree(expr.second) << std::endl;
          }
          exprs.push_back(expr.second);
        }
      }
      break;

      case LineType::DOUBLE_DOLLAR:
      case LineType::EMPTY:
      //end happily
      done = true;
      break;

      case LineType::DOUBLE_PERCENT:
      //ignore
      break;
    }
  }

  return exprs;
}

void
TLText::setup_clargs()
{
  if (m_clargs.size() != 0)
  {
    std::vector<size_t> bounds{m_clargs.size()};
    std::vector<Constant> dims
    {
      Types::Dimension::create(m_system.getDimensionIndex(U"n"))
    };

    m_argsHD = new ArrayNHD<u32string, 1>
    (
      bounds,
      dims,
      m_system,
      Types::String::create,
      Types::String::get
    );

    int i = 0;
    for (auto v : m_clargs)
    {
      (*m_argsHD)[i] = u32string(v.begin(), v.end());
      ++i;
    }

    m_system.addInputHyperdaton(U"CLARGS", m_argsHD);
  }
}

void
TLText::setup_envhd()
{
  m_envHD = new EnvHD(m_system, m_system);

  m_system.addInputHyperdaton(U"ENV", m_envHD);
}

} //namespace TLCore

} //namespace TransLucid
