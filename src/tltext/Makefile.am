bin_PROGRAMS = tltext

tltext_LDFLAGS = -lltdl -export-dynamic \
$(top_builddir)/src/libs/system/libtlsystem.la \
$(BOOST_LDFLAGS) $(BOOST_PROGRAM_OPTIONS_LIB) $(TL_LDFLAGS)

tltext_CPPFLAGS = -I$(top_srcdir)/src/include \
-DSHAREDIR=\"$(pkgdatadir)\" -DPREFIX=\"$(prefix)\" \
-Wall $(BOOST_CPPFLAGS) $(TL_CFLAGS)

tltext_SOURCES = main.cpp tltext.cpp tltext.hpp demandhd.cpp demandhd.hpp

TESTS_ENVIRONMENT = TLCORE=$(top_builddir)/src/tltext/tltext \
  TESTDIR=$(top_srcdir)/src/tltext/tests \
  RUNBINARY=$(top_srcdir)/src/tests/runbinary.sh \
  RUNTESTS=$(top_srcdir)/src/tltext/runtests.sh \
  LIBPATH=$(top_builddir)/src/libs/int:$(top_builddir)/src/libs/string $(SHELL) -x

#TESTS = tltext-tests.sh

#check-local:
#	$(top_srcdir)/src/tests/runbinary.sh $(top_srcdir)/src/tlcore/runtests.sh \
#	$(top_builddir)/src/tlcore/tlcore \
#	$(top_srcdir)/src/tlcore/tests
