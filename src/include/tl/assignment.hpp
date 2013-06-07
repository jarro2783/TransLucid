/* Assignments
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
 * @file assignment.hpp
 * Assignment declarations.
 */

#ifndef TL_ASSIGNMENT_HPP_INCLUDED
#define TL_ASSIGNMENT_HPP_INCLUDED

#include <tl/ast.hpp>
#include <tl/workshop.hpp>

#include <memory>

namespace TransLucid
{
  class Assignment
  {
    public:

    struct Definition
    {
      Tree::Expr booleanExpr;
      Tree::Expr guardExpr;
      Tree::Expr bodyExpr;
      std::shared_ptr<WS> guardWS;
      std::shared_ptr<WS> booleanWS;
      std::shared_ptr<WS> bodyWS;
    };

    Assignment(u32string name)
    : m_name(name) 
    {}

    void
    addDefinition(Definition d)
    {
      m_definitions.push_back(d);
    }

    void
    evaluate
    (
      System& s,
      Context& k
    );

    const std::vector<Definition>&
    definitions() const
    {
      return m_definitions;
    }

    private:

    u32string m_name;
    std::vector<Definition> m_definitions;
  };
}

#endif
