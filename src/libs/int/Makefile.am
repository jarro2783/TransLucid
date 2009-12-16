EXTRA_DIST = CMakeLists.txt

pkglib_LTLIBRARIES = libint.la

libint_la_SOURCES = libint.hpp libint.cpp 

libint_la_CPPFLAGS = -I$(top_srcdir)/src/include $(GLIBMM_CFLAGS) -Wall \
$(BOOST_CPPFLAGS)

libint_la_LDFLAGS = -version-info 0:0:0 -lgmpxx -lgmp
