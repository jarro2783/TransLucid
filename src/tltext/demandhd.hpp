/* tltext demand hyperdaton
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

#ifndef TLTEXT_DEMAND_HD
#define TLTEXT_DEMAND_HD

#include <tl/hyperdaton.hpp>
#include <tl/registries.hpp>

#include <vector>

namespace TransLucid
{
  namespace TLText
  {
    class DemandHD : public OutputHD
    {
      public:

      DemandHD(DimensionRegistry& dims);

      void
      put(const Tuple& k, const Constant& c);

      const Constant&
      operator()(size_t i)
      {
        return m_results.at(i);
      }

      Tuple
      variance() const;

      private:

      std::vector<Constant> m_results;
      dimension_index m_slot;
    };
  }
}

#endif
