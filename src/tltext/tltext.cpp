/* TLText application implementation.
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <tl/hyperdatons/multi_arrayhd.hpp>
#include <tl/line_tokenizer.hpp>
#include <tl/output.hpp>
#include <tl/parser_api.hpp>
#include <tl/tree_printer.hpp>
#include <tl/types/boolean.hpp>
#include <tl/types/dimension.hpp>
#include <tl/types/string.hpp>
#include <tl/types/uuid.hpp>
#include <tl/types_util.hpp>
#include <tl/system.hpp>

#include <iterator>
#include <iostream>
#include <fstream>

#include <boost/format.hpp>

#include "tltext.hpp"

#include "gettext.h"

#define _(String) gettext (String)

/**
 * @file tltext.cpp
 * The tltext application. All of the code which runs the main tltext
 * application.
 */

namespace TransLucid
{

namespace TLText
{

TLText::TLText
(
  const std::string& progname, 
  const std::string& initOut,
  bool cached
)
: 
  m_myname(progname)
 ,m_verbose(1)
 ,m_uuids(false)
 ,m_debug(false)
 ,m_cached(cached)
 ,m_is(&std::cin)
 ,m_os(&std::cout)
 ,m_error(&std::cerr)
 ,m_inputName(U"<interactive>")
 ,m_initialOut(initOut)
 ,m_system(m_cached)
 ,m_time(0)
 ,m_lastLibLoaded(0)
 ,m_deps(nullptr)
 ,m_argsHD(0)
 ,m_envHD(0)
{
  m_libtool.addSearchPath(to_u32string(std::string(PREFIX "/share/tl")));

  m_demands = new DemandHD(m_system);

  m_system.addOutputHyperdaton(U"demand", m_demands);

  //set up demand for RETURN
  m_returnhd = new DemandHD(m_system);
  m_system.addOutputHyperdaton(U"returnval", m_returnhd);
  m_system.addAssignment
  (
    Parser::Equation
    (
      U"returnval", 
      Tree::RegionExpr(
      {
        Tree::RegionExpr::Entry
        {
          Tree::DimensionExpr(U"slot"), 
          Region::Containment::IS,
          mpz_class(0)
        }
      })
      ,
      Tree::Expr(), 
      Tree::IdentExpr(U"RETURN")
    )
  );

  m_system.addEnvVars();
}

TLText::~TLText()
{
  delete m_demands;
  delete m_argsHD;
  delete m_envHD;
  delete m_returnhd;
  delete m_deps;
}

VerboseOutput
TLText::output(std::ostream& os, int level)
{
  return VerboseOutput(m_verbose, level, os);
}

void 
TLText::set_input(std::istream* is, const std::string& name)
{
  m_is = is;
  m_inputName = utf8_to_utf32(name);
}

void
TLText::add_header(const std::string& header)
{
  m_headers.push_back(header);
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

  //save some settings
  bool uuids = m_uuids;
  m_uuids = false;
  for (auto h : m_headers)
  {
    std::ifstream is(h.c_str());

    if (is)
    {
      is >> std::noskipws;
      Parser::U32Iterator begin(
        Parser::makeUTF8Iterator(std::istream_iterator<char>(is))
      );

      Parser::U32Iterator end(
        Parser::makeUTF8Iterator(std::istream_iterator<char>())
      );

      LineTokenizer tokens(begin, end);

      m_system.disableCache();
      processDefinitions(tokens, utf8_to_utf32(h));
    }
  }

  m_uuids = uuids;

  //make instant 0 happen
  m_system.go();

  *m_error << m_initialOut << std::endl;
  *m_is >> std::noskipws;

  Parser::U32Iterator begin(
    Parser::makeUTF8Iterator(std::istream_iterator<char>(*m_is))
  );

  Parser::U32Iterator end(
    Parser::makeUTF8Iterator(std::istream_iterator<char>()));

  LineTokenizer tokenizer(begin, end);

  bool done = false;
  while (!done)
  {
    m_system.disableCache();
    auto defs = processDefinitions(tokenizer, m_inputName);

    if (m_cached)
    {
      m_system.enableCache();
    }

    //only continue if the instant is valid
    if (defs.first)
    {
      std::vector<Tree::Expr> exprs;
      //only parse expressions if the right tokens were seen
      if (defs.second)
      {
        exprs = processExpressions(tokenizer, m_inputName);
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
          Tree::RegionExpr
          (
            {
              Tree::RegionExpr::Entry
              {
                Tree::DimensionExpr(U"time"), 
                Region::Containment::IS,
                mpz_class(time)
              },
              Tree::RegionExpr::Entry
              {
                Tree::DimensionExpr(U"slot"), 
                Region::Containment::IS,
                mpz_class(slot)
              }
            }
          ),
          Tree::Expr(),

          Tree::LambdaAppExpr(Tree::IdentExpr(U"canonical_print"), e)

          //Tree::AtExpr(
          //  Tree::IdentExpr(U"CANONICAL_PRINT"),
          //  Tree::TupleExpr({{Tree::DimensionExpr(U"arg0"), e}})
          //)
        ));
        ++slot;
      }

      computeDependencies();

      //run the demands
      m_system.go();

      output(*m_os, OUTPUT_STANDARD) << 
      //TRANSLATORS: verbose output, which instant we are at
        boost::format(_("// instant %1% beginning")) % time << std::endl;

      //print some stuff
      for (int s = 0; s != slot; ++s)
      {
        output(*m_os, OUTPUT_STANDARD) << 
        //TRANSLATORS: verbose output, which demand we are printing
          boost::format(_("// demand %1%")) % s << std::endl;
        const auto& c = (*m_demands)(s);
        if (c.index() == TYPE_INDEX_USTRING)
        {
          output(*m_os, OUTPUT_SILENT) << Types::String::get(c) << std::endl;
        }
        else
        {
          output(*m_error, OUTPUT_SILENT) 
            << "Error: canonical_print didn't return a string" 
            << std::endl;
          output(*m_error, OUTPUT_SILENT) << "Type index: " << c.index() 
            << std::endl;
          if (c.index() == TYPE_INDEX_SPECIAL)
          {
            output(*m_error, OUTPUT_SILENT) << "special: " << 
              get_constant<Special>(c) << std::endl;
          }
        }
      }

      output(*m_os, OUTPUT_STANDARD) << 
      //TRANSLATORS: verbose output, which instant we are at
        boost::format(_("// instant %1% end")) % time << std::endl;

      //check the return value
      const auto& ret = (*m_returnhd)(0);
      if (ret.index() != TYPE_INDEX_INTMP)
      {
        throw ReturnError(RETURN_CODE_NOT_INTMP);
      }
      else
      {
        const mpz_class& val = get_constant_pointer<mpz_class>(ret);
        if (val < 0 || val > 255)
        {
          throw ReturnError(RETURN_CODE_BOUNDS);
        }

        if (val != 0)
        {
          throw ReturnError(val.get_si());
        }
      }
    }
    else
    {
      done = true;
    }
  }
}

std::pair<bool, bool>
TLText::processDefinitions
(
  LineTokenizer& tokenizer, 
  const u32string& streamName
)
{
  bool instantValid = true;
  bool parseExpressions = true;
  bool first = true;

  bool done = false;
  while (!done)
  {
    auto line = tokenizer.next();
    switch(line.type)
    {
      case LineType::LINE:
      //parse a line with the system
      {
        #if 0
        Parser::U32Iterator lineBegin(
          Parser::makeUTF32Iterator(line.text.begin()));
        Parser::U32Iterator lineEnd(
          Parser::makeUTF32Iterator(line.text.end())
        );

        Parser::StreamPosIterator posbegin(lineBegin, streamName,
          line.line,line.character);
        Parser::StreamPosIterator posend(lineEnd);
        #endif

        try
        {
          //auto result = 
          //  m_system.parseLine(posbegin, posend, m_verbose, m_debug);
          auto result = m_system.addDeclaration(
            Parser::RawInput
            {
              streamName,
              line.line,
              line.character,
              line.text
            }
          );


          if (m_cached)
          {
            //how do I do this?
            if (result.index() == TYPE_INDEX_UUID)
            {
              m_system.cacheObject(Types::UUID::get(result));
            }
          }

          if (m_uuids)
          {
            //print out whatever we got back
            output(*m_os, OUTPUT_STANDARD) << m_system.printConstant(result)
              << std::endl;
          }
        }
        catch (TransLucid::Parser::ParseError& e)
        {
          const Parser::Position& pos = e.m_pos;
          output(*m_error, OUTPUT_SILENT) << m_myname << ":" << 
            streamName << ":" 
            << pos.line << ":" 
            << pos.character << ":" << e.what() << std::endl;
        }
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
TLText::processExpressions
(
  LineTokenizer& tokenizer, 
  const u32string& streamName
)
{
  std::vector<Tree::Expr> exprs;
  bool done = false;
  while (!done)
  {
    auto line = tokenizer.next();

    switch(line.type)
    {
      case LineType::LINE:
      //parse an expression
      {
        Parser::U32Iterator lineBegin(
          Parser::makeUTF32Iterator(line.text.begin()));
        Parser::U32Iterator lineEnd(
          Parser::makeUTF32Iterator(line.text.end())
        );

        Parser::StreamPosIterator posbegin(lineBegin, streamName,
          line.line, line.character);
        Parser::StreamPosIterator posend(lineEnd);

        try
        {
          Tree::Expr expr;
          if (m_system.parseExpression(posbegin, posend, expr))
          {
            if (m_verbose > 1)
            {
              (*m_os) << Printer::print_expr_tree(expr) << std::endl;
            }
            exprs.push_back(expr);
          }
        }
        catch(TransLucid::Parser::ParseError& e)
        {
          const auto& pos = e.m_pos;
          (*m_error) << m_myname << m_inputName << ":" << pos.line << ":" 
                     << pos.character << ":" << e.what() << std::endl;
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

  return std::move(exprs);
}

void
TLText::setup_clargs()
{
  
  m_system.addVariableDeclParsed(Parser::Equation
    {
      U"CLARGS",
      Tree::Expr(),
      Tree::Expr(),
      u32string()
    }
  );

  if (m_clargs.size() != 0)
  {
    int i = 0;
    for (auto v : m_clargs)
    {
      m_system.addVariableDeclParsed(Parser::Equation
        {
          U"CLARGS",
          Tree::TupleExpr(Tree::TupleExpr::TuplePairs
          {
            {Tree::DimensionExpr(U"arg0"), mpz_class(i)}
          }),
          Tree::Expr(),
          utf8_to_utf32(v)
        });
      ++i;
    }
  }
}

void
TLText::setup_envhd()
{
  //m_envHD = new EnvHD(m_system, m_system);

  //m_system.addInputHyperdaton(U"ENV", m_envHD);
}


void
TLText::add_argument(const u32string& arg)
{
  m_system.addEnvVar(arg, Types::Boolean::create(true));
}

void
TLText::add_argument(const u32string& arg, const u32string& value)
{
  m_system.addEnvVar(arg, Types::String::create(value));
}

void
TLText::computeDependencies()
{
  if (m_deps)
  {
    auto result = m_deps->computeDependencies();

    std::cout << "Dependencies:\n";
    for (const auto& dep : result)
    {
      std::cout << dep.first << ": ";

      for (const auto& x : std::get<0>(dep.second))
      {
        std::cout << x << " ";
      }
      std::cout << std::endl;
    }
  }
}

} //namespace TLText

} //namespace TransLucid
