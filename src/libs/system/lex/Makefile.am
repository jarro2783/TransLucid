#noinst_PROGRAMS=generate_lexer

$(bindir)/generate_lexer: lexertl.cpp

#generate_lexer_SOURCES=lexertl.cpp
BUILT_SOURCES=$(srcdir)/static_lexer.hpp
EXTRA_DIST=$(BUILT_SOURCES) lexertl.cpp
MAINTAINERCLEAN=$(BUILT_SOURCES)

generate_lexer_CPPFLAGS = -I$(top_srcdir)/src/include $(TL_CFLAGS) -Wall \
-I$(top_srcdir)/src/libs/system

$(srcdir)/static_lexer.hpp: $(srcdir)/lexertl.cpp $(srcdir)/../tl/lexer_tokens.hpp
	$(MAKE) $(AM_MAKEFLAGS) $(builddir)/generate_lexer
	$(builddir)/generate_lexer > $@
