/* The context.
   Copyright (C) 2009-2011 Jarryd Beck and John Plaice

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
 * @file context.hpp
 * The context object.
 */

#ifndef TL_CONTEXT_HPP_INCLUDED
#define TL_CONTEXT_HPP_INCLUDED

#include <tl/types.hpp>

namespace TransLucid
{
  class Context
  {
    public:

    Context();

    void
    perturb(const Tuple& t);

    /**
     * Restores to the previous context.
     * Pops one value from every dimension that exists in t.
     * @param t The tuple to restore from.
     */
    void 
    restore(const Tuple& t);

    Constant
    lookup(dimension_index dim);

    private:
    dimension_index min;
    dimension_index max;
  };
}

#endif
