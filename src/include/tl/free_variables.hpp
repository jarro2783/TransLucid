/* Finds free variables in an expression.
   Copyright (C) 2013 Jarryd Beck

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

#ifndef TL_FREE_VARIABLES_HPP_INCLUDED
#define TL_FREE_VARIABLES_HPP_INCLUDED

#include <tl/ast.hpp>
#include <tl/generic_walker.hpp>

namespace TransLucid
{
  //finds the free variables inside an expression, ignoring where clauses
  //this only works for transformed expressions, meaning that the free
  //variables are basically any identifier that appears
  class FreeVariables : private GenericTreeVisitor<FreeVariables>
  {
    public:

    using GenericTreeVisitor::operator();

    typedef void result_type;

    std::set<u32string>
    findFree(const Tree::Expr& e);

    result_type
    operator()(const Tree::IdentExpr&);

    result_type
    operator()(const Tree::WhereExpr&);

    private:

    std::set<u32string> m_free;
  };

}

#endif
