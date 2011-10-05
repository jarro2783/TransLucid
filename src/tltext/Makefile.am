bin_PROGRAMS = tltext wilfred

tltext_LDFLAGS = -lltdl -export-dynamic \
$(top_builddir)/src/libs/system/libtlsystem.la \
$(BOOST_LDFLAGS) $(BOOST_PROGRAM_OPTIONS_LIB) $(TL_LDFLAGS)

tltext_CPPFLAGS = -I$(top_srcdir)/src/include \
-DSHAREDIR=\"$(pkgdatadir)\" -DPREFIX=\"$(prefix)\" \
-Wall $(BOOST_CPPFLAGS) $(TL_CFLAGS)

wilfred_LDFLAGS = $(tltext_LDFLAGS)
wilfred_CPPFLAGS = $(tltext_CPPFLAGS)

TLTEXT_SHARED_SOURCES = tltext.cpp tltext.hpp demandhd.cpp demandhd.hpp

tltext_SOURCES = main.cpp $(TLTEXT_SHARED_SOURCES)

wilfred_SOURCES= wilfred.cpp $(TLTEXT_SHARED_SOURCES)

TESTS_ENVIRONMENT = TLTEXT=$(top_builddir)/src/tltext/tltext \
  TESTDIR=$(top_srcdir)/src/tltext/tests \
  RUNBINARY=$(top_srcdir)/src/tests/runbinary.sh \
  RUNTESTS=$(top_srcdir)/src/tltext/runtests.sh \
  LIBPATH=$(top_builddir)/src/libs/int:$(top_builddir)/src/libs/string $(SHELL) -x

tltextdatadir = $(pkgdatadir)/tltext
dist_tltextdata_DATA = header.tl

EXTRA_DIST = CMakeLists.txt

TESTS = tltext-tests.sh

#check-local:
#	$(top_srcdir)/src/tests/runbinary.sh $(top_srcdir)/src/tlcore/runtests.sh \
#	$(top_builddir)/src/tlcore/tlcore \
#	$(top_srcdir)/src/tlcore/tests
