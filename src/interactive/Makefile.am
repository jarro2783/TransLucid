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
bin_PROGRAMS = tl-interactive

tl_interactive_CPPFLAGS = -I$(top_srcdir)/src/include \
$(BOOST_CPPFLAGS)
tl_interactive_LDFLAGS = \
$(top_builddir)/src/libs/interpreter/libtlsystem.la \
$(BOOST_LDFLAGS) $(BOOST_PROGRAM_OPTIONS_LIB)

tl_interactive_CXXFLAGS = -O2 -Wall

tl_interactive_SOURCES = main.cpp interactive.hpp interactive.cpp \
parser.hpp
