#   Copyright (C) 2009--2012 Jarryd Beck, 2013 John Plaice
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

SUBDIRS = lex tl . po

EXTRA_DIST = CMakeLists.txt po/CMakeLists.txt

lib_LTLIBRARIES = libtlsystem.la

libtlsystem_la_LDFLAGS = \
  -version-info 4:0:0 \
  $(TL_LDFLAGS)
libtlsystem_la_LIBADD = \
  -lgmpxx -lgmp \
  -lltdl $(TL_LIBS) $(BOOST_LDFLAGS)

libtlsystem_la_SOURCES = \
  assignment.cpp ast.cpp bestfit.cpp builtin_types.cpp \
  cache.cpp charset.cpp chi.cpp context.cpp datadef.cpp \
  dependencies.cpp dimtranslator.cpp equation.cpp \
  eval_workshops.cpp function.cpp \
  hyperdatons/arrayhd.cpp hyperdatons/envhd.cpp hyperdatons/filehd.cpp \
  internal_strings.cpp lexertl.cpp lexer_util.cpp library.cpp \
  line_tokenizer.cpp opdef.cpp parser.cpp range.cpp region.cpp rename.cpp \
	semantic_transform.cpp \
  system.cpp system_util.cpp tree_printer.cpp tree_rewriter.cpp \
  tree_to_wstree.cpp \
  tyinf/constraint_graph.cpp tyinf/type_inference.cpp \
  types/function_type.cpp types/numbers.cpp types/union.cpp \
  types.cpp utility.cpp uuid.cpp workshop_builder.cpp

libtlsystem_la_CPPFLAGS = \
  -I$(top_srcdir)/src/include \
  -DPKGLIBDIR=\"$(pkglibdir)\" -Wall $(BOOST_CPPFLAGS) $(TL_CFLAGS) \
  -DTRANSLATE_DOMAIN=\"libtl\" -DLOCALEDIR=\"$(localedir)\"

systemdatadir = $(pkgdatadir)
dist_systemdata_DATA = system_header.tl

PO_SUBDIRS=po
include ../../../Makefile.pot
