bin_PROGRAMS = tlcore

tlcore_LDFLAGS = -lltdl -export-dynamic \
$(top_builddir)/src/libs/system/libtlsystem.la \
$(BOOST_LDFLAGS) $(BOOST_PROGRAM_OPTIONS_LIB) $(TL_LDFLAGS)

tlcore_CPPFLAGS = -I$(top_srcdir)/src/include \
-DSHAREDIR=\"$(pkgdatadir)\" -Wall $(BOOST_CPPFLAGS) $(TL_CFLAGS)

tlcore_SOURCES = main.cpp tlcore.cpp tlcore.hpp
