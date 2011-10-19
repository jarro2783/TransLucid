dnl   Copyright (C) 2009, 2010 Jarryd Beck and John Plaice
dnl
dnl   This file is part of TransLucid.
dnl
dnl   TransLucid is free software; you can redistribute it and/or modify
dnl   it under the terms of the GNU General Public License as published by
dnl   the Free Software Foundation; either version 3, or (at your option)
dnl   any later version.
dnl
dnl   TransLucid is distributed in the hope that it will be useful,
dnl   but WITHOUT ANY WARRANTY; without even the implied warranty of
dnl   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
dnl   GNU General Public License for more details.
dnl
dnl   You should have received a copy of the GNU General Public License
dnl   along with TransLucid; see the file COPYING.  If not see
dnl   <http://www.gnu.org/licenses/>.

AC_PREREQ([2.61])
AC_INIT([tl], [0.0.80], [jarrydb@cse.unsw.edu.au])
AC_CONFIG_SRCDIR([src/libs/system/system.cpp])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIR([m4])

AM_INIT_AUTOMAKE([dist-bzip2 -Wno-portability])

m4_ifdef([AM_SILENT_RULES],[AM_SILENT_RULES([yes])])

# Checks for programs.
AC_PROG_CXX

AC_LIBTOOL_DLOPEN
AC_PROG_LIBTOOL

AC_LANG([C++])

GCC_MIN_VERSION=`cat $srcdir/checks/gcc_min_version`
[$srcdir/checks/generate_gcc_min_version_check]

# check for the correct version of gcc
AC_MSG_CHECKING([for gcc >= $GCC_MIN_VERSION])
GCC_CHECK_TEXT=`cat $srcdir/checks/version.cpp`
AC_COMPILE_IFELSE([AC_LANG_SOURCE([$GCC_CHECK_TEXT])], 
  [AC_MSG_RESULT([yes])],
  [AC_MSG_ERROR([gcc version $GCC_MIN_VERSION or greater is required])]
)

# Checks for libraries.

AX_BOOST_BASE([1.46])
AX_BOOST_PROGRAM_OPTIONS
AX_BOOST_THREAD

TL_LDFLAGS="-std=gnu++0x"
TL_CFLAGS="-std=gnu++0x -fno-strict-aliasing -DBOOST_SPIRIT_UNICODE \
  -DBOOST_MPL_LIMIT_LIST_SIZE=30 -DBOOST_MPL_CFG_NO_PREPROCESSED_HEADERS"

AC_SUBST(TL_LDFLAGS)
AC_SUBST(TL_CFLAGS)

# Checks for typedefs, structures, and compiler characteristics.

AC_CHECK_SIZEOF([wchar_t],,[#include <wchar.h>])
if test "0$ac_cv_sizeof_wchar_t" -lt "4"; then
   AC_MSG_ERROR([Sizeof wchar_t < 4, your system is braindead, I refuse to work])
fi

# Do we build tlweb
AC_ARG_ENABLE([tlweb],
  AS_HELP_STRING([--enable-tlweb], [Whether to build the Web server or not, 
  defaults to no. 
  Set to static or dynamic depending on whether you want the Web server built
  as a static executable or dynamically linked, defaults to dynamic.]),
  [if test "x$enable_tlweb" = xyes; then
    enable_tlweb=dynamic
  fi
  if test "x$enable_tlweb" != xstatic && test "x$enable_tlweb" != xdynamic; 
  then
    AC_MSG_FAILURE([--enable-tlweb must be given with static or dynamic])
  fi
  ],
  [enable_tlweb=no]
)

# check that if tlweb is static, that disable static is not set
if test "x$enable_tlweb" == "xstatic" && test "x$enable_static" = xno; then
  AC_MSG_FAILURE([--enable-tlweb=static and --enable-static=no])
fi

AC_ARG_ENABLE([webprefix],
  AS_HELP_STRING([--webprefix=WEBPREFIX], [Set where to install the web
  server]),
  [WEBPREFIX=$enable_webprefix],
  )

AC_SUBST(WEBPREFIX)

AM_CONDITIONAL([WANT_TLWEB], [test "x$enable_tlweb" != xno])
AM_CONDITIONAL([TLWEB_STATIC], [test "x$enable_tlweb" = xstatic])
AM_CONDITIONAL([WEB_SEPARATE_PREFIX], [test "x$WEBPREFIX" != "x"])

# Check if we can build documentation
DX_INIT_DOXYGEN([TransLucid], [doc])
DX_DOT_FEATURE([ON])

if test -n "$DX_DOT"; then
  HAVE_DOT="YES"
else
  HAVE_DOT="NO"
fi

AC_SUBST(HAVE_DOT)

AC_CONFIG_FILES([
                 translucid.pc
                 Makefile
                 checks/Makefile
                 doc/Makefile
                 doc/Doxyfile
                 src/Makefile
                 src/include/Makefile
                 src/include/tl/Makefile
                 src/include/tl/detail/Makefile
                 src/include/tl/hyperdatons/Makefile
                 src/include/tl/types/Makefile
                 src/libs/Makefile
                 src/libs/system/Makefile
                 src/tests/Makefile
                 src/tltext/Makefile
                 ])
AC_OUTPUT
