/* Replaces free variables with #hidden
   Copyright (C) 2011,2012 Jarryd Beck

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

#include <tl/free_variables.hpp>

namespace TransLucid
{

std::set<u32string>
FreeVariables::findFree(const Tree::Expr& e)
{
  m_free.clear();

  apply_visitor(*this, e);

  return m_free;
}

void
FreeVariables::operator()(const Tree::IdentExpr& e)
{
  m_free.insert(e.text);
}

void
FreeVariables::operator()(const Tree::WhereExpr& e)
{
  apply_visitor(*this, e.e);

  for (const auto& d : e.dims)
  {
    apply_visitor(*this, d.second);
  }
}

}
