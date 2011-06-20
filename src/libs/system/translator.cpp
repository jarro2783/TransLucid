/* Translate between representations of code.
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

#include <tl/equation_parser.hpp>
#include <tl/expr_parser.hpp>
#include <tl/header_parser.hpp>
#include <tl/instant_parser.hpp>
#include <tl/line_parser.hpp>
#include <tl/translator.hpp>
#include <tl/tuple_parser.hpp>
#include <tl/types.hpp>
#include <tl/utility.hpp>

#include <boost/spirit/home/qi/parser.hpp>
#include <boost/spirit/include/qi_parse.hpp>
#include <boost/spirit/include/qi_parse_attr.hpp>

namespace TransLucid
{

namespace detail
{
  struct AllParsers
  {
    AllParsers(Parser::Header& h)
    : m_lexer(m_errors)
    , m_expr(h, m_lexer)
    , m_equation(m_lexer)
    , m_tuple(m_lexer)
    , m_header_grammar(m_lexer)
    , m_header_binary(m_lexer)
    , m_header_string(m_lexer)
    , m_header_unary(m_lexer)
    , m_line(m_lexer, m_equation)
    , m_instant(m_lexer, m_line)
    {
      m_expr.set_tuple(m_tuple);
      m_tuple.set_expr(m_expr);

      m_equation.set_expr(m_expr);
      m_equation.set_tuple(m_tuple);

      m_header_grammar.set_expr(m_expr);
    }

    Parser::Errors m_errors;
    Lexer::tl_lexer m_lexer;
    Parser::ExprGrammar<Parser::iterator_t> m_expr;
    Parser::EquationGrammar<Parser::iterator_t> m_equation;
    Parser::TupleGrammar<Parser::iterator_t> m_tuple;
    Parser::HeaderGrammar<Parser::iterator_t> m_header_grammar;
    Parser::HeaderBinopGrammar<Parser::iterator_t> m_header_binary;
    Parser::HeaderStringGrammar<Parser::iterator_t> m_header_string;
    Parser::HeaderUnopGrammar<Parser::iterator_t> m_header_unary;
    Parser::LineGrammar<Parser::iterator_t> m_line;
    Parser::InstantGrammar<Parser::iterator_t> m_instant;
  };
}

Translator::Translator(System& system)
:  m_header(0)
  ,m_system(system)
  ,m_compiler(&system)
  ,m_nextLib(0)
{
  try
  {
    m_header = new Parser::Header;

    m_parsers = new detail::AllParsers(*m_header);
  }
  catch (...)
  {
    cleanup();
    throw;
  }
}

Translator::~Translator()
{
  cleanup();
}

#if 0
WS*
Translator::translate_expr(const u32string& u32s)
{
  Lexer::base_iterator_t pos
  (
    Parser::makeUTF32Iterator(u32s.begin()),
    Parser::makeUTF32Iterator(u32s.end())
  );
  Tree::Expr e;
  
  bool success = boost::spirit::lex::tokenize_and_parse(
    pos,
    Lexer::base_iterator_t(),
    m_parsers->m_lexer,
    m_parsers->m_expr,
    e);

  //std::cerr << "pos at: ";
  //std::cerr << *pos << std::endl;

  if (success == false)
  {
    std::cerr << "failed to parse" << std::endl;
  }

  if (pos != Lexer::base_iterator_t())
  {
    std::cerr << "didn't read all input" << std::endl;
  }

  #if 0
  if (r == false || pos != Parser::iterator_t()) 
  {
    return 0;
  }
  #endif

  m_lastExpr = e;

  return m_compiler.compile_top_level(e);
}
#endif

std::pair<bool, std::pair<Parser::Equation, Parser::DeclType>>
Translator::parseEquation
(
  Parser::U32Iterator& begin, 
  const Parser::U32Iterator& end
)
{
  std::pair<Parser::Equation, Parser::DeclType> eqn;
  bool success = boost::spirit::lex::tokenize_and_parse(
    begin,
    end,
    m_parsers->m_lexer,
    m_parsers->m_equation,
    eqn
  );

  return std::make_pair(success, eqn);
}

bool
Translator::parse_header(const u32string& s)
{
  Lexer::base_iterator_t pos(
    Parser::makeUTF32Iterator(s.begin()),
    Parser::makeUTF32Iterator(s.end()));
  std::vector<int> test;

  bool success = boost::spirit::lex::tokenize_and_parse(
    pos,
    Lexer::base_iterator_t(),
    m_parsers->m_lexer,
    (m_parsers->m_header_grammar)(boost::phoenix::ref(*m_header))
    );

  //load any more libraries specified in the header
  loadLibraries();

  return (success && pos == Lexer::base_iterator_t());
}

std::pair<bool, Parser::BinopHeader>
Translator::parseHeaderBinary
(
  Parser::U32Iterator& begin, 
  const Parser::U32Iterator& end
)
{
  Parser::BinopHeader op;
  bool success = boost::spirit::lex::tokenize_and_parse(
    begin,
    end,
    m_parsers->m_lexer,
    m_parsers->m_header_binary,
    op
  );

  return std::make_pair(success, op);
}

std::pair<bool, Parser::UnopHeader>
Translator::parseHeaderUnary
(
  Parser::U32Iterator& begin, 
  const Parser::U32Iterator& end
)
{
  Parser::UnopHeader op;
  bool success = boost::spirit::lex::tokenize_and_parse(
    begin,
    end,
    m_parsers->m_lexer,
    m_parsers->m_header_unary,
    op
  );

  return std::make_pair(success, op);
}

std::pair<bool, u32string>
Translator::parseHeaderString
(
  Parser::U32Iterator& begin, 
  const Parser::U32Iterator& end
)
{
  u32string arg;
  #if 0
  bool success = boost::spirit::lex::tokenize_and_parse(
    begin,
    end,
    m_parsers->m_lexer,
    m_parsers->m_header_string,
    arg
  );
  #endif

  Lexer::lexer_type::iterator_type iter = 
    m_parsers->m_lexer.begin(begin, end);
  Lexer::lexer_type::iterator_type last = m_parsers->m_lexer.end();

  bool success = boost::spirit::qi::parse(
    iter,
    last,
    m_parsers->m_header_string,
    arg
  );

  std::cerr << "after parser" << std::endl;

  if (*iter != m_parsers->m_lexer.dblsemi_.id())
  {
    success = false;
  }
  else
  {
    std::cerr << "we saw a ;;, continue..." << std::endl;
    ++begin;
  }

  return std::make_pair(success, arg);
}

void
Translator::loadLibraries()
{
  for(; m_nextLib != m_header->libraries.size(); ++m_nextLib) 
  {
    const u32string& l = m_header->libraries.at(m_nextLib);
    loadLibrary(l);
  }
}

void
Translator::cleanup()
{
  delete m_header;
}

std::string Translator::EquationIterator::print() const
{
  return Parser::printEquation(m_iter->second);
}

void
Translator::loadLibrary(const u32string& s)
{
  m_lt.loadLibrary(s, m_system);
}

bool
Translator::parseInstant
(
  Parser::U32Iterator& begin,
  const Parser::U32Iterator& end,
  Parser::InstantFunctor endInstant
)
{
  Parser::Instant instant;

  Lexer::lexer_type::iterator_type iter = 
    m_parsers->m_lexer.begin(begin, end);
  Lexer::lexer_type::iterator_type last = m_parsers->m_lexer.end();

  bool success = boost::spirit::qi::parse(
    iter,
    last,
    (m_parsers->m_instant)(boost::phoenix::ref(endInstant))
  );

  if (success)
  {
    //endInstant(instant);
  }
  else
  {
    std::cerr << "failed to parse instant" << std::endl;
  }

  return success;
}

}
