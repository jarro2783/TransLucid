/* Equations (ident = expr)
   Copyright (C) 2009--2012 Jarryd Beck

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

/** @file equation.cpp
 * All of the code related to equations. The main part of bestfitting
 * and the management of equations is done here.
 */

#include <tl/equation.hpp>
#include <tl/eval_workshops.hpp>
#include <tl/output.hpp>
#include <tl/range.hpp>
#include <tl/types/demand.hpp>
#include <tl/types/function.hpp>
#include <tl/types/range.hpp>
#include <tl/types/tuple.hpp>
#include <tl/types_util.hpp>
#include <tl/utility.hpp>

#include <vector>

#define XSTRING(x) STRING(x)
#define STRING(x) #x

namespace TransLucid
{

namespace
{
  template <typename Container>
  Constant
  evaluateBestselect(Context& k, Container&& results, const Constant& fn)
  {
    Constant currentVal = results.front();

    auto iter = results.begin();
    ++iter;

    while (iter != results.end())
    {
      Constant fn2 = applyFunction<FUN_VALUE>(k, fn, currentVal);
      currentVal = applyFunction(k, fn2, *iter);
      ++iter;
    }

    return currentVal;
  }
}

VariableWS::VariableWS(const u32string& name, System& system)
: m_name(name), m_bestfit(this, system)
{
  m_bestfit.setName(U"variable: " + m_name);
}

void
VariableWS::addEquation(uuid id, Parser::Variable eqn, int time, 
  ScopePtr scope)
{
  m_bestfit.addEquation(id, eqn, time, scope);
}

void
VariableWS::addEquation(uuid id, Parser::RawInput input, int time,
  ScopePtr scope)
{
  m_bestfit.addEquation(id, input, time, scope);
}

bool
VariableWS::del(uuid id, size_t time)
{
  return m_bestfit.del(id, time);
}

bool
VariableWS::repl(uuid id, size_t time, Parser::Line line)
{
  return m_bestfit.repl(id, time, line);
}

Tree::Expr
VariableWS::group(const std::list<EquationDefinition>& lines)
{
  //all the definitions in lines are guaranteed to have a parsed
  //definition

  //create a conditional best fitter of all these equations
  Tree::ConditionalBestfitExpr best;
  best.name = m_name;
  for (auto& l : lines)
  {
    auto eqn = get<Parser::Variable>(l.parsed().get());
    //auto eqn = get<Parser::Equation>(l.parsed().get());

    if (eqn == nullptr)
    {
      throw "internal compiler error: " __FILE__ ":" XSTRING(__LINE__);
    }

    best.declarations.push_back(std::make_tuple
    (
      l.start(),
      std::get<1>(eqn->eqn),
      std::get<2>(eqn->eqn),
      std::get<3>(eqn->eqn)
    ));

#if 0
    best.declarations.push_back(std::make_tuple
    (
      l.start(),
      std::get<1>(*eqn),
      std::get<2>(*eqn),
      std::get<3>(*eqn)
    ));
#endif
  }

  return best;
}

}
