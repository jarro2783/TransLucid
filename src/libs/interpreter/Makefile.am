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

lib_LTLIBRARIES = libtlinterpreter.la

libtlinterpreter_la_LDFLAGS = -version-info 0:0:0 \
$(BOOST_LDFLAGS) $(TL_LDFLAGS)
libtlinterpreter_la_LIBADD = -lrt -lgmpxx -lgmp $(BOOST_THREAD_LIB) \
-lltdl

libtlinterpreter_la_SOURCES = interpreter.cpp expr_parser.cpp \
library.cpp types.cpp \
dimtranslator.cpp \
range.cpp parser.cpp \
builtin_types.cpp header_type.cpp equation.cpp \
cache.cpp utility.cpp compiled_functors.cpp \
expr_compiler.cpp consthd.cpp printer.cpp container.cpp set_types.cpp \
translator.cpp ast.cpp tuple_parser.cpp

libtlinterpreter_la_CPPFLAGS = -I$(top_srcdir)/src/include \
-DPKGLIBDIR=\"$(pkglibdir)\" -Wall $(BOOST_CPPFLAGS) $(TL_CFLAGS)
