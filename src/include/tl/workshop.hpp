/* Lots of things are workshops.
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

#ifndef WORKSHOP_HPP_INCLUDED
#define WORKSHOP_HPP_INCLUDED

#include <tl/context.hpp>
#include <tl/types.hpp>
//#include <tl/system.hpp>

namespace TransLucid
{
  class Thread;
  class Delta;

  class WS
  {
    public:
    virtual ~WS() {}

    virtual Constant
    operator()(Context& k) = 0;

    virtual Constant
    operator()(Context& kappa, Context& delta) = 0;

    virtual Constant
    operator()(Context& kappa, Delta& d, const Thread& w, size_t t) = 0;
  };
} //namespace TransLucid

#endif // WORKSHOP_HPP_INCLUDED
