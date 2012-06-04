/* Function workshop code
   Copyright (C) 2012 Jarryd Beck

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

/**
 * @file system.hpp
 * The System.
 */

#include <tl/function.hpp>

#define STRING(x) #x
#define XSTRING(x) STRING(x)

namespace TransLucid
{

Constant
FunctionWS::operator()(Context& k)
{
  return m_bestfit(k);
}

Constant
FunctionWS::operator()(Context& kappa, Context& delta)
{
  return m_bestfit(kappa, delta);
}

void
FunctionWS::addEquation(uuid id, Parser::RawInput input, int time)
{
}

bool 
FunctionWS::del(uuid id, size_t time)
{
  return m_bestfit.del(id, time);
}

bool 
FunctionWS::repl(uuid id, size_t time, Parser::Line line)
{
  return m_bestfit.repl(id, time, line);
}

Tree::Expr
FunctionWS::group(const std::list<EquationDefinition>& defs)
{
  //for each function definition, rename the parameters in the guard
  //and then build up a conditional bestfit from the resulting expression
  //also make sure that the types of parameters are consistent for all
  //definitions

  std::vector<Parser::FnDecl::ArgType> params;

  for (auto& eqn : defs)
  {
    auto& line = *eqn.parsed();
    auto fundecl = get<Parser::FnDecl>(&line);

    if (fundecl == nullptr)
    {
      throw "Internal compiler error: " __FILE__ ":" XSTRING(__LINE__);
    }

    if (params.size() == 0)
    {
      //create the parameters
      for (auto p : fundecl->args)
      {
        params.push_back(p.first);
      }
    }
    else
    {
      //check that the parameters are consistent
      auto paramIter = params.begin();
      auto declIter = fundecl->args.begin();
      while (declIter != fundecl->args.end() && paramIter != params.end())
      {
        if (*paramIter != declIter->first)
        {
          //throw a parse error here
        }
      }
    }
  }
}

}
