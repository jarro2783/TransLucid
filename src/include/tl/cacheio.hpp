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

#ifndef TL_CACHE_IO_HPP_INCLUDED
#define TL_CACHE_IO_HPP_INCLUDED

#include <tl/cache.hpp>
#include <tl/hyperdaton.hpp>

namespace TransLucid
{
  class CacheIO : public IOHD
  {
    public:

    void
    put(const Context& k, const Constant& c);

    Constant
    get(const Context& k) const;

    private:
    mutable Cache m_cache;
  };
}

#endif
