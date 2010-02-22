noinst_PROGRAMS = single echo typewriter

single_SOURCES = single.cpp

echo_SOURCES = echo.cpp

typewriter_SOURCES = typewriter.cpp

AM_CPPFLAGS = -I$(top_srcdir)/src/include $(GLIBMM_CFLAGS) \
-DSHAREDIR=\"$(pkgdatadir)\" -Wall $(BOOST_CPPFLAGS) $(TL_CFLAGS)

AM_LDFLAGS = -lpthread $(GLIBMM_LIBS) -lltdl -export-dynamic \
$(top_builddir)/src/libs/interpreter/libtlinterpreter.la \
$(BOOST_LDFLAGS) $(BOOST_PROGRAM_OPTIONS_LIB) $(TL_LDFLAGS)
