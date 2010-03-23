bin_PROGRAMS = tl-interactive

tl_interactive_CPPFLAGS = -I$(top_srcdir)/src/include \
$(BOOST_CPPFLAGS)
tl_interactive_LDFLAGS = \
$(top_builddir)/src/libs/interpreter/libtlinterpreter.la \
$(BOOST_LDFLAGS) $(BOOST_PROGRAM_OPTIONS_LIB)

tl_interactive_CXXFLAGS = -O2 -Wall

tl_interactive_SOURCES = main.cpp interactive.hpp interactive.cpp \
parser.hpp
