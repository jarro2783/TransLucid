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

#include <tl/translator.hpp>
#include <tl/equation_parser.hpp>
#include <tl/header_parser.hpp>
#include <tl/expr_parser.hpp>
#include <tl/tuple_parser.hpp>
#include <boost/spirit/home/qi/parser.hpp>
#include <tl/types.hpp>
#include <tl/utility.hpp>

namespace TransLucid
{

namespace
{
  namespace qi = boost::spirit::qi;

  template <typename Iterator>
  class EquationSetGrammar
  : public qi::grammar<Iterator, std::vector<Parser::Equation>()>
  {
    public:
    template <typename T, typename TokenDef>
    EquationSetGrammar(T& t, TokenDef& tok)
    : EquationSetGrammar::base_type(equations)
    {
      using boost::phoenix::push_back;
      using namespace qi::labels;

      eqn %= t;

      one_equation = eqn
        [
          _val = _1
        ]
      ;

      equations %= *(one_equation >> tok.dblsemi_) >> qi::eoi
        //[
        //  push_back(_val, _1)
        //]
      ;

      BOOST_SPIRIT_DEBUG_NODE(one_equation);
      //BOOST_SPIRIT_DEBUG_NODE(equations);
    }

    private:

    qi::rule
    <
      Iterator,
      std::vector<Parser::Equation>()
    > equations;

    qi::rule
    <
      Iterator,
      Parser::Equation()
    >
      one_equation,
      eqn
    ;
  };
}

namespace detail
{
  struct AllParsers
  {
    AllParsers(Parser::Header& h)
    : m_lexer(m_errors)
    , m_expr(h, m_lexer)
    , m_equation(m_lexer)
    , m_header_grammar(m_lexer)
    , m_header_binary(m_lexer)
    , m_header_string(m_lexer)
    , m_header_unary(m_lexer)
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

    #if 0
    m_lexer = new Lexer::tl_lexer;
    //m_expr = new Parser::ExprGrammar<Parser::iterator_t>(*m_header, *m_lexer);
    m_expr = create_expr_grammar(*m_header, *m_lexer);
    m_equation = new Parser::EquationGrammar<Parser::iterator_t>(*m_lexer);
    //m_tuple = new Parser::TupleGrammar<Parser::iterator_t>;
    m_tuple = Parser::create_tuple_grammar();
    m_header_grammar = new Parser::HeaderGrammar<Parser::iterator_t>(*m_lexer);

    m_expr->set_tuple(*m_tuple);
    m_tuple->set_expr(*m_expr);

    m_equation->set_expr(*m_expr);
    m_equation->set_tuple(*m_tuple);

    m_header_grammar->set_expr(*m_expr);
    #endif
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

std::pair<bool, Parser::Equation>
Translator::parseEquation
(
  Parser::U32Iterator& begin, 
  const Parser::U32Iterator& end
)
{
  Parser::Equation eqn;
  bool success = boost::spirit::lex::tokenize_and_parse(
    begin,
    end,
    m_parsers->m_lexer,
    m_parsers->m_equation,
    eqn
  );

  return std::make_pair(success, eqn);
}

PTEquationVector
Translator::translate_equation_set(const u32string& s)
{
  Lexer::base_iterator_t pos(Lexer::base_iterator_t(
    Parser::makeUTF32Iterator(s.begin()),
    Parser::makeUTF32Iterator(s.end())));

  EquationSetGrammar<Parser::iterator_t> equation_set
    (m_parsers->m_equation, m_parsers->m_lexer);
  std::vector<Parser::Equation> parsedEquations;

  bool success = boost::spirit::lex::tokenize_and_parse(
    pos,
    Lexer::base_iterator_t(),
    m_parsers->m_lexer,
    equation_set,
    parsedEquations);

  if (!success)
  {
    throw "failed parsing equations";
  }

  PTEquationVector equations;

  BOOST_FOREACH(auto& v, parsedEquations)
  {
    WS* context = m_compiler.compile_for_equation(std::get<1>(v));
    WS* boolean = m_compiler.compile_for_equation(std::get<2>(v));
    WS* e = m_compiler.compile_for_equation(std::get<3>(v));
    equations.push_back(
      std::make_pair(
      v,
      TranslatedEquation(
        to_u32string(std::get<0>(v)),
        context,
        boolean,
        e)
      )
    );

    #if 0

    typedef std::back_insert_iterator<std::string> out_iter;
    Printer::ExprPrinter<out_iter> print_grammar;
    std::string generated;
    std::back_insert_iterator<std::string> outit(generated);

    std::cerr << "parsed equation: " << std::endl;
    std::cerr << "name: " << utf32_to_utf8(to_u32string(std::get<0>(v)))
    << std::endl;

    Printer::karma::generate(outit, print_grammar, std::get<1>(v));
    std::cerr << "guard: " << generated << std::endl;

    generated.clear();
    Printer::karma::generate(outit, print_grammar, std::get<2>(v));
    std::cerr << "boolean: " << generated << std::endl;

    generated.clear();
    Printer::karma::generate(outit, print_grammar, std::get<3>(v));
    std::cerr << "equation: " << generated << std::endl;
    #endif
  }

  return equations;
}

std::list<std::pair<uuid, Parser::Equation>>
Translator::translate_and_add_equation_set(const u32string& s)
{
  PTEquationVector equations = translate_equation_set(s);
  std::list<std::pair<uuid, Parser::Equation>> added;

  BOOST_FOREACH(auto& ptv, equations)
  {
    auto v = ptv.second;
    uuid id = m_system.addEquation(std::get<0>(v), 
      GuardWS(std::get<1>(v), std::get<2>(v)), std::get<3>(v));

    m_uuidParsedEqns.insert(std::make_pair(id, ptv.first));
    added.push_back(std::make_pair(id, ptv.first));
  }

  return added;
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
  bool success = boost::spirit::lex::tokenize_and_parse(
    begin,
    end,
    m_parsers->m_lexer,
    m_parsers->m_header_string,
    arg
  );

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
  m_lt.loadLibrary(s, &m_system);
}

}
