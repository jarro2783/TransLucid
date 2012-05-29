/* Equations (ident = expr)
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

#include <tl/bestfit.hpp>
#include "tl/parser.hpp"

namespace TransLucid
{

//TODO finish this
//the choice of how to best fit will very much depend on properties of the
//system
TaggedConstant CompileBestFit::operator()(const Tuple& k)
{
  BestFit* b = nullptr;
  BestFit* old = m_bestFittable->setBestFit(b);
  if (old != this) {
  }

  return (*b)(k);
}

bool
UnparsedEquations::del(uuid id, int time)
{
}

std::vector<ParsedDefinition>
UnparsedEquations::parse(System& system, Context& k)
{
  Parser::Parser p(system);

  std::vector<ParsedDefinition> defs;
  while (m_parsed != m_definitions.size())
  {
    auto& definition = m_definitions.at(m_parsed);
    auto& text = definition.definition();

    Parser::U32Iterator ubegin(Parser::makeUTF32Iterator(text.text.begin()));
    Parser::U32Iterator uend(Parser::makeUTF32Iterator(text.text.end()));

    Parser::StreamPosIterator posbegin(ubegin, text.source, 
      text.line, text.character);
    Parser::StreamPosIterator posend(uend);

    Parser::LexerIterator lexbegin(posbegin, posend, k, 
      system.lookupIdentifiers());
    Parser::LexerIterator lexend = lexbegin.makeEnd();

    Parser::Line result;
    bool success = p.parse_decl(lexbegin, lexend, result);

    if (!success)
    {
    }

    ++m_parsed;
  }

  return defs;
}

void
BestfitGroup::compile(Context& k)
{
  auto defs = m_unparsed.parse(m_system, k);

  for (auto& definition : defs)
  {
    m_definitions.push_back(definition);

    auto iter = m_uuids.find(definition.id());

    if (iter == m_uuids.end())
    {
      m_uuids[definition.id()] = 
        std::list<size_t>{m_definitions.size() - 1};
    }
    else
    {
      iter->second.push_back(m_definitions.size() - 1);
    }
  }

  //add in the extra definitions for that which has changed
  if (m_changes.size() > m_evaluators.size())
  {
    size_t change = m_evaluators.size();

    while (change != m_changes.size())
    {
      //the start is where that change was
      int start = m_changes[change];
      //there is no end
      int end = -1;

      if (change != 0)
      {
        //the previous should end one before the current time
        m_evaluators[change-1].end = m_changes[change] - 1;
      }

      //compile one group of definitions
      Tree::Expr expression = compileInstant(m_changes[change]);

      ++change;
    }
  }

  //compile the new stuff together
  //Tree::Expr expr = m_grouper->group(defs);
}

Tree::Expr
BestfitGroup::compileInstant(int time)
{
  //look for everything that is valid at time and compile it into one
  //expression

  std::vector<ParsedDefinition> valid;
  for (auto i = m_definitions.begin(); i != m_definitions.end(); ++i)
  {
    if (i->start() <= time && (i->end() == -1 || i->end() < time))
    {
      valid.push_back(*i);
    }
  }
}

Constant
BestfitGroup::operator()(Context& k)
{
  if (m_unparsed.has_unparsed())
  {
    try
    {
      compile(k);
    }
    catch (...)
    {
      //special parse error
    }
  }
}

} //namespace TransLucid
