bin_PROGRAMS = tltext

if WANT_TLWEB
  bin_PROGRAMS += tlweb matrixweb
endif

TLTEXT_LIBS = $(top_builddir)/src/libs/system/libtlsystem.la \
$(BOOST_LDFLAGS) $(BOOST_PROGRAM_OPTIONS_LIB) -lltdl

tltext_LDFLAGS = $(TLTEXT_LIBS) $(TL_LDFLAGS) -export-dynamic

tltext_CPPFLAGS = -I$(top_srcdir)/src/include \
-DSHAREDIR=\"$(pkgdatadir)\" -DPREFIX=\"$(prefix)\" \
-Wall $(BOOST_CPPFLAGS) $(TL_CFLAGS)

if TLWEB_STATIC
  TLWEB_STATIC_FLAGS = -all-static
  matrixweb_LDFLAGS=-all-static
endif

tlweb_LDFLAGS = $(TLTEXT_LIBS) $(TLWEB_STATIC_FLAGS)
tlweb_CPPFLAGS = $(tltext_CPPFLAGS)

matrixweb_CPPFLAGS=-std=gnu++11

if WEB_SEPARATE_DATA
  tlweb_CPPFLAGS += -DHEADERDIR=\"$(WEBHEADER)\"
else
  tlweb_CPPFLAGS += -DHEADERDIR=\"$(datadir)/tl/tltext\"
endif

TLTEXT_SHARED_SOURCES = tltext.cpp tltext.hpp demandhd.cpp demandhd.hpp

tltext_SOURCES = main.cpp $(TLTEXT_SHARED_SOURCES)

tlweb_SOURCES= tlweb.cpp $(TLTEXT_SHARED_SOURCES)

matrixweb_SOURCES = matrixweb.cpp

TESTS_ENVIRONMENT = TLTEXT=$(top_builddir)/src/tltext/tltext \
  TESTDIR=$(top_srcdir)/src/tltext/tests \
  RUNBINARY=$(top_srcdir)/src/tests/runbinary.sh \
  RUNTESTS=$(top_srcdir)/src/tltext/runtests.sh \
  LIBPATH=$(top_builddir)/src/libs/int:$(top_builddir)/src/libs/string $(SHELL) -x

tltextdatadir = $(pkgdatadir)/tltext
dist_tltextdata_DATA = header.tl

EXTRA_DIST = CMakeLists.txt tltext-tests.sh runtests.sh

TESTS = tltext-tests.sh

#check-local:
#	$(top_srcdir)/src/tests/runbinary.sh $(top_srcdir)/src/tlcore/runtests.sh \
#	$(top_builddir)/src/tlcore/tlcore \
#	$(top_srcdir)/src/tlcore/tests
