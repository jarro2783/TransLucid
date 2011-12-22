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

SUBDIRS = lex tl

EXTRA_DIST = CMakeLists.txt

lib_LTLIBRARIES = libtlsystem.la

libtlsystem_la_LDFLAGS = -version-info 0:0:0 \
$(BOOST_LDFLAGS) $(TL_LDFLAGS)
libtlsystem_la_LIBADD = -lrt -lgmpxx -lgmp $(BOOST_THREAD_LIB) \
-lltdl

libtlsystem_la_SOURCES = ast.cpp bestfit.cpp builtin_types.cpp \
  charset.cpp context.cpp \
  dimtranslator.cpp equation.cpp \
  eval_workshops.cpp \
  hyperdatons/arrayhd.cpp hyperdatons/envhd.cpp hyperdatons/filehd.cpp \
  internal_strings.cpp lexertl.cpp lexer_util.cpp library.cpp \
  line_tokenizer.cpp parser.cpp range.cpp rename.cpp \
  system.cpp tree_printer.cpp tree_to_wstree.cpp \
  types.cpp utility.cpp uuid.cpp workshop_builder.cpp

libtlsystem_la_CPPFLAGS = -I$(top_srcdir)/src/include \
-DPKGLIBDIR=\"$(pkglibdir)\" -Wall $(BOOST_CPPFLAGS) $(TL_CFLAGS)

systemdatadir = $(pkgdatadir)
dist_systemdata_DATA = system_header.tl
