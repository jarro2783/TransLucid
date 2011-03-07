#   Copyright (C) 2009-2011 Jarryd Beck and John Plaice
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
EXTRA_DIST=CMakeLists.txt

check_PROGRAMS = expressions equations printing lexer iterator_test

AM_LDFLAGS = -lpthread -lltdl -export-dynamic \
$(top_builddir)/src/libs/system/libtlsystem.la \
$(BOOST_LDFLAGS) $(BOOST_PROGRAM_OPTIONS_LIB) $(TL_LDFLAGS)

AM_CPPFLAGS = -I$(top_srcdir)/src/include \
-DSHAREDIR=\"$(pkgdatadir)\" -Wall $(BOOST_CPPFLAGS) $(TL_CFLAGS)

expressions_SOURCES = expressions.cpp

equations_SOURCES = equations.cpp

printing_SOURCES = printing.cpp

lexer_SOURCES = lexer.cpp

iterator_test_SOURCES = iterator.cpp

TESTS_ENVIRONMENT = LIBRARY_PATH=$(top_builddir)/src/libs/int \
  RUNBINARY=$(top_srcdir)/src/tests/runbinary.sh $(SHELL) -x
TESTS = $(patsubst %,run_%.sh,$(check_PROGRAMS))

run_%.sh: runtest
	sed 's&PROGRAM_NAME&$(top_builddir)/src/tests/$*&' < \
	  $(top_srcdir)/src/tests/runtest > $@
	chmod +x $@
