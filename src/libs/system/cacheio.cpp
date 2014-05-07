/* Cache IO.
   Copyright (C) 2014 Jarryd Beck

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

#include <tl/cacheio.hpp>
#include <tl/fixed_indexes.hpp>

namespace TransLucid
{

void
CacheIO::put(const Context& k, const Constant& c)
{
  Delta d;
  k.fillDelta(d);

  m_cache.set(k, d, c);
}

Constant
CacheIO::get(const Context& k) const
{
  Delta d;
  k.fillDelta(d);
  auto r = m_cache.get(k, d);

  if (r.index() == TYPE_INDEX_DEMAND || r.index() == TYPE_INDEX_CALC)
  {
    return Types::Special::create(SP_UNDEF);
  }

  return r;
}

}
