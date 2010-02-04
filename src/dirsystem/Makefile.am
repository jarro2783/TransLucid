EXTRA_DIST=CMakeLists.txt

bin_PROGRAMS = tl

tl_LDFLAGS = -lpthread $(GLIBMM_LIBS) -lltdl -export-dynamic \
$(top_builddir)/src/libs/interpreter/libtlinterpreter.la \
$(BOOST_LDFLAGS) $(BOOST_PROGRAM_OPTIONS_LIB) $(TL_LDFLAGS)

tl_CPPFLAGS = -I$(top_srcdir)/src/include $(GLIBMM_CFLAGS) \
-DSHAREDIR=\"$(pkgdatadir)\" -Wall $(BOOST_CPPFLAGS) $(TL_CFLAGS)

tl_SOURCES = main.cpp directory_system.cpp directory_system.hpp
