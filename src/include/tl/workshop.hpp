/* Lots of things are workshops.
   Copyright (C) 2009--2013 Jarryd Beck

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
  class Thread
  {
  };

  class WS
  {
    public:
    virtual ~WS() {}

    virtual Constant
    operator()(Context& k) = 0;

    virtual Constant
    operator()(Context& kappa, Context& delta) = 0;

    virtual TimeConstant
    operator()(Context& kappa, Delta& d, const Thread& w, size_t t) = 0;
  };

  namespace detail
  {
    template <typename... Args>
    struct EvalRetType
    {
      typedef Constant type;
    };

    template <>
    struct EvalRetType<Delta, Thread, size_t>
    {
      typedef TimeConstant type;
    };

    inline
    Constant
    cached_return(Constant c)
    {
      return c;
    }

    inline
    Constant
    cached_return(Constant c, const Context&)
    {
      return c;
    }

    inline
    TimeConstant
    cached_return(Constant c, Delta&, const Thread&, size_t t)
    {
      return std::make_pair(t, c);
    }
  }
} //namespace TransLucid

#endif // WORKSHOP_HPP_INCLUDED
