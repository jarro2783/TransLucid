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
#include <tl/utility.hpp>
#include <tl/workshop_builder.hpp>

#include "tl/parser.hpp"

static constexpr size_t LINEAR_SEARCH_MAX_SIZE = 7;

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

void
BestfitGroup::parse(Context& k)
{
  Parser::Parser p(m_system);

  while (m_parsed != m_definitions.size())
  {
    auto& definition = m_definitions.at(m_parsed);

    if (definition.raw() != nullptr)
    {
      auto& text = *definition.raw();

      Parser::U32Iterator ubegin(Parser::makeUTF32Iterator(text.text.begin()));
      Parser::U32Iterator uend(Parser::makeUTF32Iterator(text.text.end()));

      Parser::StreamPosIterator posbegin(ubegin, text.source, 
        text.line, text.character);
      Parser::StreamPosIterator posend(uend);

      Parser::LexerIterator lexbegin(posbegin, posend, k, 
        m_system.lookupIdentifiers());
      Parser::LexerIterator lexend = lexbegin.makeEnd();

      Parser::Line result;
      bool success = p.parse_decl(lexbegin, lexend, result);

      if (!success)
      {
      }
      else
      {
        definition.setParsed(result);
      }
    }

    ++m_parsed;
  }
}

void
BestfitGroup::compile(Context& k)
{
  parse(k);

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

      //make this into a workshop
      std::shared_ptr<WS> evaluator = compileExpression(expression);

      m_evaluators.push_back(CompiledDefinition{start, end, evaluator});

      ++change;
    }
  }
}

std::shared_ptr<WS>
BestfitGroup::compileExpression(const Tree::Expr& expr)
{
  //fixup the ast
  Tree::Expr fixed = m_system.fixupTreeAndAdd(expr);

  //compile the tree into a workshop
  WorkshopBuilder compile(&m_system);
  std::shared_ptr<WS> ws(compile.build_workshops(fixed));

  return ws;
}

Tree::Expr
BestfitGroup::compileInstant(int time)
{
  //look for everything that is valid at time and compile it into one
  //expression

  std::list<Parser::Line> valid;
  for (auto i = m_definitions.begin(); i != m_definitions.end(); 
    ++i)
  {
    if (i->start() <= time && (i->end() == -1 || i->end() < time))
    {
      valid.push_back(*i->parsed());
    }
  }

  return m_grouper->group(valid);
}

Constant
BestfitGroup::operator()(Context& k)
{
  if (m_parsed != m_definitions.size())
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

  return evaluate(k);
}

Constant
BestfitGroup::evaluate(Context& k)
{
  auto dimtime = k.lookup(DIM_TIME);
  int time = 0;

  if (dimtime.index() == TYPE_INDEX_INTMP)
  {
    time = Types::Intmp::get(dimtime).get_si();
  }

  //first check the last definition
  auto& last = m_evaluators.back();
  if (time >= last.start && (last.end == -1 || time <= last.end))
  {
    return (*last.evaluator)(k);
  }

  //make sure that it's not bigger than the last and smaller than the first
  if (time > last.start || time < m_evaluators.front().start)
  {
    return Types::Special::create(SP_UNDEF);
  }

  //otherwise do a linear search backwards if small, binary search if big
  if (m_evaluators.size() <= LINEAR_SEARCH_MAX_SIZE)
  {
    for (auto i = m_evaluators.rbegin(); i != m_evaluators.rend(); ++i)
    {
      if (time >= i->start && time <= i->end)
      {
        return (*i->evaluator)(k);
      }
    }
    //if we got here then something broke
    return Types::Special::create(SP_UNDEF);
  }
  else
  {
    //binary search the rest
    auto begin = m_evaluators.begin();
    auto end = m_evaluators.end();
    --end;
    auto current = begin + (end - begin) / 2;

    //this is guaranteed to find a match now
    while (!(time >= current->start && time <= current->end))
    {
      if (time < current->start)
      {
        current = begin + (current - begin) / 2;
      }
      else
      {
        current = current + (end - current) / 2;
      }
    }

    //current is the match
    return (*current->evaluator)(k);
  }
}

} //namespace TransLucid
