SUBDIRS=. po

bin_PROGRAMS = tltext

if WANT_TLWEB
  bin_PROGRAMS += tlweb matrixweb
endif

TLTEXT_LIBS = $(top_builddir)/src/libs/system/libtlsystem.la \
-lgmp

tltext_LDFLAGS = $(TL_LDFLAGS) -export-dynamic
tltext_LDADD = $(TLTEXT_LIBS)

tltext_CPPFLAGS = -I$(top_srcdir)/src/include \
-DSHAREDIR=\"$(pkgdatadir)\" -DPREFIX=\"$(prefix)\" \
-Wall $(TL_CFLAGS) -DLOCALEDIR=\"$(localedir)\" \
-DTRANSLATE_DOMAIN=\"tltext\"

if TLWEB_STATIC
  TLWEB_STATIC_FLAGS = -all-static -pthread
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
  LIBPATH=$(top_builddir)/src/libs/int:$(top_builddir)/src/libs/string \
  EXTRA_ARGS="--no-builtin-header --header=$(top_srcdir)/src/tltext/header.tl" \
  $(SHELL) -x

tltextdatadir = $(pkgdatadir)/tltext
dist_tltextdata_DATA = header.tl header-tyinf.tl

EXTRA_DIST = CMakeLists.txt tltext-tests.sh runtests.sh gettext.h tests \
po/CMakeLists.txt examples caching

TESTS = tltext-tests.sh

#check-local:
#	$(top_srcdir)/src/tests/runbinary.sh $(top_srcdir)/src/tlcore/runtests.sh \
#	$(top_builddir)/src/tlcore/tlcore \
#	$(top_srcdir)/src/tlcore/tests

PO_SUBDIRS=po
include ../../Makefile.pot
