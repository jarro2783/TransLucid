$(builddir)/generate_lexer: lexertl.cpp
	$(CXX) $(AM_CXXFLAGS) $(CXXFLAGS) $(TL_CFLAGS) -I$(top_srcdir)/src/include \
  -I$(srcdir)/.. $(LDFLAGS) $< -o $@

BUILT_SOURCES=$(srcdir)/static_lexer.hpp
EXTRA_DIST=$(BUILT_SOURCES) lexertl.cpp CMakeLists.txt
MAINTAINERCLEAN=$(BUILT_SOURCES)

generate_lexer_CPPFLAGS = -I$(top_srcdir)/src/include $(TL_CFLAGS) -Wall \
-I$(top_srcdir)/src/libs/system

$(srcdir)/static_lexer.hpp: $(srcdir)/lexertl.cpp $(srcdir)/../tl/lexer_tokens.hpp
	$(MAKE) $(AM_MAKEFLAGS) $(builddir)/generate_lexer
	$(builddir)/generate_lexer > $@
