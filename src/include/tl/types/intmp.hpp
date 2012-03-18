/* The intmp type.
   Copyright (C) 2011 Jarryd Beck and John Plaice

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

#ifndef TYPES_INTMP_HPP_INCLUDED
#define TYPES_INTMP_HPP_INCLUDED

#include <tl/gmpxx_fwd.hpp>
#include <tl/types.hpp>

namespace TransLucid
{
  namespace Types
  {
    namespace Intmp
    {
      Constant
      create(const Constant& text);

      Constant
      create(const mpz_class& v);

      Constant
      create(int v);

      const mpz_class&
      get(const Constant& c);

      bool 
      equality(const Constant& lhs, const Constant& rhs);

      size_t
      hash(const Constant& c);

      Constant
      print(const Constant& c);

      bool
      less(const Constant& lhs, const Constant& rhs);
    }
  }
}

#endif
