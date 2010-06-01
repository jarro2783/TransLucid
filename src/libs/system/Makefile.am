#   Copyright (C) 2009, 2010 Jarryd Beck and John Plaice
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

lib_LTLIBRARIES = libtlsystem.la

libtlsystem_la_LDFLAGS = -version-info 0:0:0 \
$(BOOST_LDFLAGS) $(TL_LDFLAGS)
libtlsystem_la_LIBADD = -lrt -lgmpxx -lgmp $(BOOST_THREAD_LIB) \
-lltdl

libtlsystem_la_SOURCES = ast.cpp bestfit.cpp builtin_types.cpp cache.cpp \
  compiled_functors.cpp container.cpp dimtranslator.cpp equation.cpp \
  expr_compiler.cpp expr_parser.cpp header.cpp library.cpp \
  parser.cpp printer.cpp range.cpp set_types.cpp \
  system.cpp translator.cpp tuple_parser.cpp types.cpp \
  utility.cpp valuehd.cpp

libtlsystem_la_CPPFLAGS = -I$(top_srcdir)/src/include \
-DPKGLIBDIR=\"$(pkglibdir)\" -Wall $(BOOST_CPPFLAGS) $(TL_CFLAGS)