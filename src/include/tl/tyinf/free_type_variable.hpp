/* Types for type inference.
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

#include <tl/tyinf/type_traverse.hpp>

#include <set>

namespace TransLucid
{
  namespace TypeInference
  {
    class FreeTypeVariables : private GenericTypeTraverser<FreeTypeVariables>
    {
      public:

      typedef void result_type;

      using GenericTypeTraverser::operator();

      result_type
      operator()(TypeVariable v)
      {
        m_free.insert(v);
      }

      std::set<TypeVariable>
      free(const Type& t)
      {
        m_free.clear();
        apply_visitor(*this, t);

        return m_free;
      }

      private:
      std::set<TypeVariable> m_free;
    };
  }
}
