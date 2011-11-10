noinst_PROGRAMS=generate_lexer

generate_lexer_SOURCES=lexertl.cpp
BUILT_SOURCES=$(srcdir)/static_lexer.hpp
EXTRA_DIST=$(BUILT_SOURCES)
MAINTAINERCLEAN=$(BUILT_SOURCES)

generate_lexer_CPPFLAGS = -I$(top_srcdir)/src/include $(TL_CFLAGS) -Wall \
-I$(top_srcdir)/src/libs/system

$(srcdir)/static_lexer.hpp: $(builddir)/generate_lexer
	$(builddir)/generate_lexer > $@
