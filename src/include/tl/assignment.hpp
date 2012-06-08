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

#include <tl/workshop.hpp>

#include <memory>

namespace TransLucid
{
  class Assignment
  {
    public:

    Assignment(u32string name)
    : m_name(name) 
    {}

    void
    addDefinition
    (
      std::shared_ptr<WS> guard,
      std::shared_ptr<WS> boolean,
      std::shared_ptr<WS> expr
    )
    {
      m_definitions.push_back(std::make_tuple(guard, boolean, expr));
    }

    void
    evaluate
    (
      System& s,
      Context& k
    );

    private:

    typedef std::tuple
    <
      std::shared_ptr<WS>,
      std::shared_ptr<WS>,
      std::shared_ptr<WS>
    >
    Definition;

    u32string m_name;
    std::vector<Definition> m_definitions;
  };
}

#endif
