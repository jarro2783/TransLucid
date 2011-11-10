/* Workshop type, stores a pointer to workshop in a Constant.
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

#ifndef TL_TYPES_WORKSHOP_HPP_INCLUDED
#define TL_TYPES_WORKSHOP_HPP_INCLUDED

#include <tl/workshop.hpp>

namespace TransLucid
{

  class WorkshopType
  {
    public:
    WorkshopType(WS* ws)
    : m_ws(ws)
    {
    }

    WS* ws() const
    {
      return m_ws;
    }

    private:
    WS* m_ws;
  };

  namespace Types
  {
    namespace Workshop
    {
      Constant
      create(const WS* ws);

      const WorkshopType&
      get(const Constant& c);

      bool 
      equality(const Constant& lhs, const Constant& rhs);

      size_t
      hash(const Constant& c);
    }
  }
}

#endif
