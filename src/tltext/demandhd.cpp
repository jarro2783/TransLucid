/* tltext demand hyperdaton
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

#include <iostream>

#include <gmpxx.h>

#include <tl/fixed_indexes.hpp>
#include <tl/range.hpp>
#include <tl/types/range.hpp>
#include <tl/types_util.hpp>

#include "demandhd.hpp"

namespace TransLucid
{

namespace TLText
{

DemandHD::DemandHD(DimensionRegistry& dims)
: OutputHD(1)
{
  m_slot = dims.getDimensionIndex(U"slot");
}

void
DemandHD::put(const Context& k, const Constant& c)
{
  const Constant& v = k.lookup(m_slot);

  if (v.index() != TYPE_INDEX_INTMP)
  {
    //error, shouldn't happen, just ignore
  }
  else
  {
    size_t slot = get_constant_pointer<mpz_class>(v).get_ui();

    if (m_results.size() <= slot)
    {
      //let's just allocate twice as much because the most common case is
      //probably filling up in slots 1, 2, 3...
      m_results.resize((slot+1)*2);
    }

    m_results.at(slot) = c;
  }
}

Region
DemandHD::variance() const
{
  mpz_class lhs = 0;

  return Region
  {
    {
      {m_slot, 
        {
          Region::Containment::IN,
          Types::Range::create(Range(&lhs, nullptr))
        }
      }
    }
  };
}

void
DemandHD::commit()
{
  //nothing to commit in the demand HD
}

} //namespace TLText

} //namespace TransLucid
