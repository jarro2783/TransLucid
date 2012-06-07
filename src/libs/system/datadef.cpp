/* Data declarations code.
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
 * @file datadef.cpp
 * Data and constructor declarations.
 */

#include <tl/datadef.hpp>

namespace TransLucid
{

Constant
ConsDefWS::operator()(Context& k)
{
  return m_bestfit(k);
}

Constant
ConsDefWS::operator()(Context& kappa, Context& delta)
{
  return m_bestfit(kappa, delta);
}

Tree::Expr
ConsDefWS::group(const std::list<EquationDefinition>& defs)
{
}

void
ConsDefWS::addEquation(uuid id, Parser::RawInput input, int time)
{
  m_bestfit.addEquation(id, input, time);
}

bool 
ConsDefWS::del(uuid id, size_t time)
{
  return m_bestfit.del(id, time);
}

bool 
ConsDefWS::repl(uuid id, size_t time, Parser::Line line)
{
}

}
