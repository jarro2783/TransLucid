/* The special type.
   Copyright (C) 2011--2013 Jarryd Beck

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

#ifndef TL_TYPES_SPECIAL_HPP_INCLUDED
#define TL_TYPES_SPECIAL_HPP_INCLUDED

#include <tl/types.hpp>

namespace TransLucid
{
  namespace Types
  {
    namespace Special
    {
      Constant
      create(TransLucid::Special s);

      Constant
      create(const Constant& s);

      Constant
      print(const Constant& c);
    }
  }
}

#endif


