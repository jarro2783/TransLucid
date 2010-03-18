/* Everything is a hyperdaton.
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

#ifndef HYPERDATON_HPP_INCLUDED
#define HYPERDATON_HPP_INCLUDED

#include <tl/types.hpp>
//#include <tl/interpreter.hpp>

namespace TransLucid
{
  class HD
  {
    public:
    virtual ~HD() {}

    virtual TaggedValue
    operator()(const Tuple& k) = 0;

    virtual uuid
    addExpr(const Tuple& k, HD* h)
    {
      return nil_uuid();
    }
  };
} //namespace TransLucid

#endif // HYPERDATON_HPP_INCLUDED
