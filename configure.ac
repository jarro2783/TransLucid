#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.61])
AC_INIT([tl], [0.6.0], [jarrydb@cse.unsw.edu.au])
AC_CONFIG_SRCDIR([src/dirsystem/main.cpp])
AC_CONFIG_HEADERS([config.h])

AM_INIT_AUTOMAKE([dist-bzip2])

AC_CONFIG_TESTDIR(tests, [src])

# Checks for programs.
AC_PROG_CXX

AC_LIBTOOL_DLOPEN
AC_PROG_LIBTOOL

# Checks for libraries.

AX_BOOST_BASE([1.39])
AX_BOOST_PROGRAM_OPTIONS
AX_BOOST_THREAD

PKG_CHECK_MODULES([GLIBMM], [glibmm-2.4],
[
   AC_SUBST([GLIBMM_CFLAGS])
   AC_SUBST([GLIBMM_LIBS])
],
[
   AC_MSG_ERROR([glibmm is required])
])

# Checks for header files.

# Checks for typedefs, structures, and compiler characteristics.

AC_CHECK_SIZEOF([wchar_t],,[#include <wchar.h>])
if test "0$ac_cv_sizeof_wchar_t" -lt "4"; then
   AC_MSG_ERROR([Sizeof wchar_t < 4, your system is braindead, I refuse to work])
fi

# Checks for library functions.

AC_CONFIG_FILES([Makefile
                 doc/Makefile
                 doc/Doxyfile
                 src/Makefile
                 src/include/Makefile
                 src/include/tl/Makefile
                 src/libs/Makefile
                 src/libs/int/Makefile
                 src/libs/interpreter/Makefile
                 src/libs/printer/Makefile
                 src/dirsystem/Makefile
                 src/interactive/Makefile
                 tests/Makefile
                 tests/atlocal
                 tests/sanity/Makefile
                 tests/simple/Makefile
                 tests/operations/Makefile
                 tests/range/Makefile])
AC_OUTPUT
