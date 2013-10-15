#   Copyright (C) 2009--2013 Jarryd Beck
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

ACLOCAL_AMFLAGS = -I m4
SUBDIRS= po src checks doc dist cmake
EXTRA_DIST=CMakeLists.txt INSTALL-tl config.h.cmake

PO_SUBDIRS=src
include Makefile.pot

doc:
if DX_COND_doc
if DX_COND_dot
	make -C doc doc
else
  echo dot not installed: cannot build docs
endif
else
  echo doxygen not installed: cannot build docs
endif
  
.PHONY: doc
