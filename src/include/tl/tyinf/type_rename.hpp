/* Type scheme rewriter.
   Copyright (C) 2013 Jarryd Beck

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

#ifndef TL_TYPE_RENAME_HPP_INCLUDED
#define TL_TYPE_RENAME_HPP_INCLUDED

#include <tl/tyinf/type_traverse.hpp>

namespace TransLucid
{
  namespace TypeInference
  {
    class Rename
    {
      public:

      typedef Type result_type;

      Rename(FreshTypeVar& fresh)
      : m_fresh(fresh)
      {
      }

      result_type
      operator()(TypeVariable v) const
      {
      }

      private:
      FreshTypeVar& m_fresh;
    };
  }
}

#endif
