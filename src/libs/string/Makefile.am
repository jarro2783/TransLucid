#   Copyright (C) 2009, 2010 Jarryd Beck
#
#   This file is part of TransLucid.
#
#   TransLucid is free software; you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation; either version 3, or (at your option)
#   any later version.
#
#   TransLucid is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#   GNU General Public License for more details.
#
#   You should have received a copy of the GNU General Public License
#   along with TransLucid; see the file COPYING.  If not see
#   <http://www.gnu.org/licenses/>.
EXTRA_DIST = CMakeLists.txt

pkglib_LTLIBRARIES = libstring.la

libstring_la_SOURCES = libstring.hpp libstring.cpp 

libstring_la_CPPFLAGS = -I$(top_srcdir)/src/include -Wall \
$(BOOST_CPPFLAGS) $(TL_CFLAGS)

libstring_la_LDFLAGS = -version-info 0:0:0 -lgmpxx -lgmp $(TL_LDFLAGS)
