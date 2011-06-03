/* Physical hyperdatons.
   Copyright (C) 2011 Jarryd Beck.

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

#ifndef PHYSICAL_WSS_HPP_INCLUDED
#define PHYSICAL_WSS_HPP_INCLUDED

#include <tl/workshop.hpp>

namespace TransLucid
{
  class PhysicalWS : public WS
  {
    public:

    virtual void
    setValue(const Constant& c, const Tuple& k) = 0;
  };

  //stores a single value no matter the context
  class SingleValuePhysicalWS : public PhysicalWS
  {
    public:
    void
    setValue(const Constant& c, const Tuple& k)
    {
      m_value = c;
    }

    const Constant&
    get() const
    {
      return m_value;
    }

    Constant&
    get()
    {
      return m_value;
    }

    private:
    Constant m_value;
  };

  class ArrayPhysicalWS : public PhysicalWS
  {
  };
}

#endif
