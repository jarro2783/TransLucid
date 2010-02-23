EXTRA_DIST=CMakeLists.txt

noinst_PROGRAMS = expressions equations

AM_LDFLAGS = -lpthread $(GLIBMM_LIBS) -lltdl -export-dynamic \
$(top_builddir)/src/libs/interpreter/libtlinterpreter.la \
$(BOOST_LDFLAGS) $(BOOST_PROGRAM_OPTIONS_LIB) $(TL_LDFLAGS)

AM_CPPFLAGS = -I$(top_srcdir)/src/include $(GLIBMM_CFLAGS) \
-DSHAREDIR=\"$(pkgdatadir)\" -Wall $(BOOST_CPPFLAGS) $(TL_CFLAGS)

expressions_SOURCES = expressions.cpp

equations_SOURCES = equations.cpp
