/* Loading external libraries.
   Copyright (C) 2009, 2010 Jarryd Beck and John Plaice

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

#ifndef LIBRARY_HPP_INCLUDED
#define LIBRARY_HPP_INCLUDED

#include <tl/types.hpp>
#include <ltdl.h>
#include <list>
#include <tl/hyperdaton.hpp>

namespace TransLucid
{

  typedef void (*library_loader)(WS*);

  class Libtool
  {
    public:
    Libtool();

    ~Libtool();

    void
    loadLibrary(const u32string& name, WS* system);

    void
    addSearchPath(const u32string& path);

    private:
    std::list<u32string> m_searchDirs;
  };
} //namespace TransLucid

#endif // LIBRARY_HPP_INCLUDED
