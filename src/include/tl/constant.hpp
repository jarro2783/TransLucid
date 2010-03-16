/* TODO: Give a descriptor.
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

#ifndef CONSTANT_HPP_INCLUDED
#define CONSTANT_HPP_INCLUDED

#include <tl/hyperdaton.hpp>
#include <tl/interpreter.hpp>
#include <map>

namespace TransLucid
{

  //this hyperdaton varies in the type and value dimension
  //it can be added to by adding a parser for a specific type
  //it can be used by putting k = [type : t, value : v]
  class ConstantHD : public HD
  {
    public:

    ConstantHD(Interpreter& i)
    : m_i(i)
    {
    }

    TaggedValue
    operator()(const Tuple& k);

    void
    addExpr(const Tuple& k, AST::Expr* e);

    private:
    Interpreter& m_i;

    std::map<u32string, HD*> m_build;
  };

}

#endif // CONSTANT_HPP_INCLUDED
