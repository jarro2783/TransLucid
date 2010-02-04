EXTRA_DIST = CMakeLists.txt

pkglib_LTLIBRARIES = libprinter.la

libprinter_la_SOURCES = printer.cpp

libprinter_la_CPPFLAGS = -I$(top_srcdir)/src/include $(GLIBMM_CFLAGS) -Wall \
$(BOOST_CPPFLAGS) $(TL_CFLAGS)

libprinter_la_LDFLAGS = -version-info 0:0:0 $(TL_LDFLAGS)
