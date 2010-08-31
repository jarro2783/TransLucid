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

#include "tlcore.hpp"
#include <tl/expr_parser.hpp>
#include <tl/equation_parser.hpp>
#include <tl/tuple_parser.hpp>
#include <tl/system.hpp>
#include <tl/expr_compiler.hpp>
#include <tl/header_parser.hpp>
#include <tl/tree_printer.hpp>
#include <iterator>
#include <iostream>

/**
 * @file tlcore.cpp
 * The tlcore application. All of the code which runs the main tlcore
 * application.
 */

namespace TransLucid
{

namespace TLCore
{

template <typename Iterator>
class Grammar : 
  public Parser::qi::grammar<Iterator, Parser::SkipGrammar<Iterator>>
{
  public:
  /**
   * Construct a tlcore Grammar object.
   * @param system The translucid system.
   * @param exprs The expression list to add to.
   * @param reactive If it is a reactive system.
   * @param demands If we are using demands.
   * @param evaluate The evaluator to inform of events.
   * @todo Remove exprs because we don't use it anymore.
   */
  Grammar(SystemHD& system, ExprList& exprs, bool reactive, bool demands,
    Evaluator& evaluate
  )
  : Grammar::base_type(r_program)
   ,m_expr(m_header)
   ,m_system(system)
   ,m_exprs(exprs)
   ,m_evaluator(evaluate)
  {
    namespace ph = boost::phoenix;
    using namespace boost::spirit::qi::labels;

    m_tuple.set_expr(m_expr);
    m_expr.set_tuple(m_tuple);
    m_eqn.set_expr(m_expr);
    m_eqn.set_tuple(m_tuple);
    m_header_parser.set_expr(m_expr);

    m_header.delimiter_start_symbols.add(
      to_unsigned_u32string(u32string(U"\"")).c_str(),
      Parser::Delimiter(U"ustring", U'\"', U'\"'));
    m_header.delimiter_start_symbols.add(
      to_unsigned_u32string(u32string(U"\'")).c_str(),
      Parser::Delimiter(U"uchar", '\'', '\''));

    if (reactive)
    {
      r_program = 
        (
          r_onetime 
          [
            ph::bind(&evaluateInstant, ph::ref(m_evaluator))
          ]
          % Parser::qi::lit(literal("$$"))
        )
        
      >> -Parser::qi::lit(literal("$$"))
      > Parser::qi::eoi;
    }
    else
    {
      r_program = (r_onetime > Parser::qi::eoi)
        [
          ph::bind(&evaluateInstant, ph::ref(m_evaluator))
        ];
    }

    r_onetime =
      -m_header_parser(ph::ref(m_header))
        [ph::bind(&postHeader, ph::ref(m_evaluator), ph::ref(m_header))]
    >> literal("%%")
    > r_eqns
    > r_demands_conditional
    > literal("%%")
    > r_exprs
    ;

    r_eqns = 
      *(
         (m_eqn > literal(";;"))
         [
           ph::bind(&addEquation, ph::ref(m_system), _1, ph::ref(m_evaluator))
         ]
       )
    ;

    r_exprs = 
      *(
         (m_expr > literal(";;"))
         [
           ph::bind(&addExpression, _1, ph::ref(m_evaluator))
         ]
       )
    ;

    r_demands = 
    *(
        literal('(') 
      > m_expr > literal(',')
      > m_tuple > literal(')'));
    
    if (demands)
    {
      r_demands_conditional = r_demands > literal("%%");
    }
    else
    {
      r_demands_conditional = r_demands;
    }

    Parser::qi::on_error<Parser::qi::fail>
      (
        r_program,
        std::cerr
        << ph::val("Error! Expecting ")
        << _4
        << ph::val(" here: \"")
        << ph::construct<std::string>(_3, _2)
        << ph::val("\"")
        << std::endl
      );

    r_program.name("r_program");
    r_eqns.name("r_eqns");
    m_header_parser.name("header_parser");
    BOOST_SPIRIT_DEBUG_NODE(r_program);
    BOOST_SPIRIT_DEBUG_NODE(r_exprs);
    BOOST_SPIRIT_DEBUG_NODE(r_eqns);
    BOOST_SPIRIT_DEBUG_NODE(r_demands);
    BOOST_SPIRIT_DEBUG_NODE(r_demands_conditional);
    BOOST_SPIRIT_DEBUG_NODE(r_onetime);

  }

  private:
  Parser::Header m_header;
  Parser::HeaderGrammar<Iterator> m_header_parser;
  Parser::ExprGrammar<Iterator> m_expr;
  Parser::EquationGrammar<Iterator> m_eqn;
  Parser::TupleGrammar<Iterator> m_tuple;

  Parser::qi::rule<Iterator, Parser::SkipGrammar<Iterator>> 
    r_program,
    r_exprs,
    r_eqns,
    r_demands,
    r_demands_conditional,
    r_onetime
  ;

  SystemHD& m_system;
  ExprList& m_exprs;
  Evaluator& m_evaluator;

  static void
  addEquation(SystemHD& system, const Parser::ParsedEquation& eqn,
    Evaluator& evaluate
  )
  {
    evaluate.addEquation(eqn);
  }
  
  static void
  addExpression(const Tree::Expr& e, Evaluator& evaluate)
  {
    evaluate.addExpression(e);
  }
  
  static void
  evaluateInstant(Evaluator& e)
  {
    e.evaluateInstant();
  }

  static void
  postHeader(Evaluator& e, const Parser::Header& header)
  {
    e.postHeader(header);
  }
};

TLCore::TLCore()
: 
  m_verbose(false)
 ,m_reactive(false)
 ,m_demands(false)
 ,m_uuids(false)
 ,m_grammar(0)
 ,m_is(&std::cin)
 ,m_os(&std::cout)
 ,m_compiler(&m_system)
 ,m_time(0)
 ,m_lastLibLoaded(0)
{
  m_skipper = new Parser::SkipGrammar<Parser::iterator_t>;
}

void 
TLCore::run()
{
  delete m_grammar;
  m_grammar = 
    new Grammar<Parser::iterator_t>
      (m_system, m_exprs, m_reactive, m_demands, *this);
  
  *m_is >> std::noskipws;

  Parser::iterator_t pos(Parser::U32Iterator(
    Parser::makeUTF8Iterator(
      std::istream_iterator<char>(*m_is)),
    Parser::makeUTF8Iterator(std::istream_iterator<char>())
  ));

  bool r = boost::spirit::qi::phrase_parse(
    pos,
    Parser::iterator_t(),
    *m_grammar,
    *m_skipper
  );

  if (!r && pos != Parser::iterator_t())
  {
    throw "Failed parsing";
  }

  #if 0
  typedef std::back_insert_iterator<std::string> out_iter;
  Printer::ExprPrinter<out_iter> printer;

  for (auto iter = m_exprs.begin(); iter != m_exprs.end(); ++iter)
  {
    TaggedConstant result = (iter->second)->operator()(Tuple());
    if (m_verbose)
    {
      std::string output;
      std::back_insert_iterator<std::string> outit(output);
      Printer::karma::generate(outit, printer, iter->first);
      (*m_os) << output << " -> ";
    }
    (*m_os) << result.first << std::endl;
  }
  #endif
}

std::u32string 
TLCore::read_input()
{
  std::u32string input;
  std::string buffer;

  while (!m_is->eof())
  {
    int c = m_is->get();
    if (c != EOF)
    {
      buffer += static_cast<char>(c);
    }
  }
  
  input = utf8_to_utf32(buffer);

  return input;
}

void 
TLCore::addEquation(const Parser::ParsedEquation& eqn)
{
  HD* guard = 0;
  HD* boolean = 0;
  HD* expr = 0;

  try 
  {
    guard = m_compiler.compile(std::get<1>(eqn));
    boolean = m_compiler.compile(std::get<2>(eqn));
    expr = m_compiler.compile(std::get<3>(eqn));

    m_addEquations.push_back(CompiledEquation
    ( 
      to_u32string(std::get<0>(eqn)),
      guard,
      boolean,
      expr
    ));

  }
  catch (...)
  {
    delete guard;
    delete boolean;
    delete expr;
    throw;
  }
}

void
TLCore::addExpression(const Tree::Expr& e)
{
  HD* ce = 0;
  
  try {
    ce = m_compiler.compile(e);
    if (ce == 0) {
    } else {
      m_exprs.push_back(std::make_pair(e, ce));
    }
  }
  catch (...)
  {
    delete ce;
    throw;
  }
}

void
TLCore::evaluateInstant()
{
  typedef std::back_insert_iterator<std::string> out_iter;
  Printer::ExprPrinter<out_iter> printer;

  //add the equations
  addNewEquations();
  m_addEquations.clear();

  (*m_os) << "%%" << std::endl;

  for (auto iter = m_exprs.begin(); iter != m_exprs.end(); ++iter)
  {
    tuple_t k = {{DIM_TIME, Constant(Intmp(m_time), TYPE_INDEX_INTMP)}};
    TaggedConstant result = (iter->second)->operator()(Tuple(k));
    if (m_verbose)
    {
      std::string output;
      std::back_insert_iterator<std::string> outit(output);
      Printer::karma::generate(outit, printer, iter->first);
      (*m_os) << "expr<" << output << ">" << " -> ";
    }
    (*m_os) << result.first << std::endl;
  }

  //clear the expressions at the end
  m_exprs.clear();
  ++m_time;
}

void TLCore::postHeader(const Parser::Header& header)
{
  while (m_lastLibLoaded < header.libraries.size())
  {
    m_libtool.loadLibrary(header.libraries.at(m_lastLibLoaded), &m_system);
    ++m_lastLibLoaded;
  }
}

void TLCore::addNewEquations()
{
  for 
  (
    auto iter = m_addEquations.begin();
    iter != m_addEquations.end();
    ++iter
  )
  {
    auto uuid = m_system.addExpr
    (
      Tuple
      (
        create_add_eqn_context
        (
          std::get<0>(*iter),
          std::get<1>(*iter),
          std::get<2>(*iter),
          m_time
        )
      ),
      std::get<3>(*iter)
    );

    if (m_uuids)
    {
      (*m_os) << "uuid<" << uuid << ">" << std::endl;
    }
  }
}

} //namespace TLCore

} //namespace TransLucid

