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

namespace TransLucid
{

namespace TLCore
{

template <typename Iterator>
class Grammar : 
  public Parser::qi::grammar<Iterator, Parser::SkipGrammar<Iterator>>
{
  public:
  Grammar(SystemHD& system, ExprList& exprs)
  : Grammar::base_type(r_program)
   ,m_expr(m_header)
   ,m_system(system)
   ,m_exprs(exprs)
  {
    namespace ph = boost::phoenix;
    using namespace boost::spirit::qi::labels;

    m_tuple.set_expr(m_expr);
    m_expr.set_tuple(m_tuple);
    m_eqn.set_expr(m_expr);
    m_eqn.set_tuple(m_tuple);
    m_header_parser.set_expr(m_expr);

    r_program = 
      -m_header_parser(ph::ref(m_header))
    > "%%"
    > r_eqns
    > "%%"
    > r_exprs
    > Parser::qi::eoi
    ;

    r_eqns = 
      *(
         (m_eqn > ";;")
         [
           ph::bind(&addEquation, ph::ref(m_system), _1)
         ]
       )
    ;

    r_exprs = 
      *(
         (m_expr > ";;")
         [
           ph::bind(&addExpression, ph::ref(m_system), ph::ref(m_exprs), _1)
         ]
       )
    ;
    
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
    r_eqns
  ;

  SystemHD& m_system;
  ExprList& m_exprs;

  static void
  addEquation(SystemHD& system, const Parser::ParsedEquation& eqn)
  {
    ExprCompiler compiler(&system);

    HD* guard = 0;
    HD* boolean = 0;
    HD* expr = 0;

    try 
    {
      guard = compiler.compile(std::get<1>(eqn));
      boolean = compiler.compile(std::get<2>(eqn));
      expr = compiler.compile(std::get<3>(eqn));
      
      system.addExpr
      (
        Tuple
        (
          create_add_eqn_context
          (
            to_u32string(std::get<0>(eqn)),
            guard,
            boolean
          )
        ),
        expr
      );
    
    }
    catch (...)
    {
      delete guard;
      delete boolean;
      delete expr;
      throw;
    }
  }
  
  static void
  addExpression(SystemHD& system, ExprList& exprs, const Tree::Expr& e)
  {
    ExprCompiler compiler(&system);

    HD* ce = 0;
    
    try {
      ce = compiler.compile(e);
      if (ce == 0) {
      } else {
        exprs.push_back(std::make_pair(e, ce));
      }
    }
    catch (...)
    {
      delete ce;
      throw;
    }
  }
};

TLCore::TLCore()
: 
  m_verbose(false)
 ,m_reactive(false)
 ,m_demands(false)
 ,m_is(&std::cin)
 ,m_os(&std::cout)
{
  m_grammar = 
    new Grammar<Parser::string_type::const_iterator>(m_system, m_exprs);
  m_skipper = new Parser::SkipGrammar<Parser::string_type::const_iterator>;
}

void 
TLCore::run()
{
  std::u32string input = read_input();

  Parser::string_type ws(input.begin(), input.end());
  Parser::iterator_t pos = ws.begin();
  bool r = boost::spirit::qi::phrase_parse(
    pos,
    ws.cend(),
    *m_grammar,
    *m_skipper
  );

  if (!r && pos != ws.cend())
  {
    throw "Failed parsing";
  }

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

} //namespace TLCore

} //namespace TransLucid
