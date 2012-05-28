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
AC_INIT([tl], [0.2.0], [jarrydb@cse.unsw.edu.au])
AC_CONFIG_SRCDIR([src/libs/system/system.cpp])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIR([m4])

AM_GNU_GETTEXT_VERSION([0.18.1])
AM_GNU_GETTEXT([external])

AM_INIT_AUTOMAKE([dist-bzip2 -Wno-portability])

m4_ifdef([AM_SILENT_RULES],[AM_SILENT_RULES([yes])])

# Checks for programs.
AC_PROG_CXX

AC_LIBTOOL_DLOPEN
AC_PROG_LIBTOOL

AC_LANG([C++])

GCC_MIN_VERSION=`cat $srcdir/checks/gcc_min_version`
GCC_CHECK_TEXT=`$srcdir/checks/generate_gcc_min_version_check`

# check for the correct version of gcc
AC_MSG_CHECKING([for gcc >= $GCC_MIN_VERSION])
#GCC_CHECK_TEXT=`cat $srcdir/checks/version.cpp`
AC_COMPILE_IFELSE([AC_LANG_SOURCE([$GCC_CHECK_TEXT])], 
  [AC_MSG_RESULT([yes])],
  [AC_MSG_ERROR([gcc version $GCC_MIN_VERSION or greater is required])]
)

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
if test "x$enable_tlweb" == "xstatic" && test "x$enable_static" = "xno"; then
  AC_MSG_FAILURE([--enable-tlweb=static and --enable-static=no])
fi

AC_ARG_ENABLE([webheaderdir],
  AS_HELP_STRING([--webheaderdir=WEBHEADERDIR], [Set the directory to install
  headers to]),
  [WEBHEADER=$enable_webheaderdir],
)

AC_SUBST(WEBHEADER)

# Check that sizeof long == sizeof void*, we need it in system.cpp, 
# LineAdder::operator()(const Parser::HostDecl&)
# when we convert an mpz to long with get_ui
# if this breaks on a system then we will have to do something specialised
AC_CHECK_SIZEOF([void*])
AC_CHECK_SIZEOF([long])

AC_MSG_CHECKING([that sizeof(long) == sizeof(void*)])
if test $ac_cv_sizeof_long -ne $ac_cv_sizeof_voidp; then
  AC_MSG_RESULT([no])
  AC_MSG_FAILURE([sizeof(long) != sizeof(void*) on your system, please file a
  bug report which includes your system and compiler details])
fi
AC_MSG_RESULT([yes])

# Checks for libraries.

AX_BOOST_BASE([1.46])
AX_BOOST_PROGRAM_OPTIONS

PKG_PROG_PKG_CONFIG([0.26])

# check for libraries statically
if test "x$enable_static" = "xyes"; then
  PKG_CONFIG="${PKG_CONFIG} --static"
fi

PKG_CHECK_MODULES([ICU], [icu-uc])

AC_SUBST([ICU_CFLAGS])
AC_SUBST([ICU_LIBS])

# we have to look for lots of versions of is_print in icuuc

if test "x$enable_static" = "xyes"; then

  AC_MSG_NOTICE([checking that ICU is linkable statically])
  ICU_FOUND=no

  AC_CHECK_LIB([icuuc], [u_isprint], ICU_FOUND=yes,
  ,
  [${ICU_LIBS} -static])

  if test "x$ICU_FOUND" = "xno"; then
    AC_CHECK_LIB([icuuc], [u_isprint_49], ICU_FOUND=yes,
    ,
    [${ICU_LIBS} -static])
  fi

  if test "x$ICU_FOUND" = "xno"; then
    AC_CHECK_LIB([icuuc], [u_isprint_48], ICU_FOUND=yes,
    ,
    [${ICU_LIBS} -static])
  fi

  if test "x$ICU_FOUND" = "xno"; then
    AC_CHECK_LIB([icuuc], [u_isprint_47], ICU_FOUND=yes,
    ,
    [${ICU_LIBS} -static])
  fi

  if test "x$ICU_FOUND" = "xno"; then
    AC_CHECK_LIB([icuuc], [u_isprint_46], ICU_FOUND=yes,
    ,
    [${ICU_LIBS} -static])
  fi

  if test "x$ICU_FOUND" = "xno"; then
    AC_MSG_FAILURE([Cannot find the ICU static libraries])
  fi

fi

TL_LDFLAGS="-std=gnu++11 -Wno-unused-local-typedefs"
TL_LIBS="${ICU_LIBS}"
TL_CFLAGS="-std=gnu++11 -fno-strict-aliasing ${ICU_CFLAGS}"

AC_SUBST(TL_LDFLAGS)
AC_SUBST(TL_LIBS)
AC_SUBST(TL_CFLAGS)

# Checks for typedefs, structures, and compiler characteristics.

AM_CONDITIONAL([WANT_TLWEB], [test "x$enable_tlweb" != xno])
AM_CONDITIONAL([TLWEB_STATIC], [test "x$enable_tlweb" = xstatic])
AM_CONDITIONAL([WEB_SEPARATE_DATA], [test "x$WEBHEADER" != "x"])

# Check if we can build documentation
DX_INIT_DOXYGEN([TransLucid], [doc])
DX_DOT_FEATURE([ON])

if test -n "$DX_DOT"; then
  HAVE_DOT="YES"
else
  HAVE_DOT="NO"
fi

AC_SUBST(HAVE_DOT)

AC_CONFIG_FILES([po/Makefile
                 dist/Makefile
                 src/libs/system/po/Makefile.in 
                 src/tltext/po/Makefile.in
                 translucid.pc
                 Makefile
                 checks/Makefile
                 doc/Makefile
                 doc/Doxyfile
                 src/Makefile
                 src/include/Makefile
                 src/include/lexertl/Makefile
                 src/include/lexertl/containers/Makefile
                 src/include/lexertl/parser/Makefile
                 src/include/lexertl/parser/tokeniser/Makefile
                 src/include/lexertl/parser/tree/Makefile
                 src/include/lexertl/partition/Makefile
                 src/include/tl/Makefile
                 src/include/tl/detail/Makefile
                 src/include/tl/hyperdatons/Makefile
                 src/include/tl/types/Makefile
                 src/libs/system/lex/Makefile
                 src/libs/system/tl/Makefile
                 src/libs/Makefile
                 src/libs/system/Makefile
                 src/tests/Makefile
                 src/tltext/Makefile
                 cmake/Makefile
                 cmake/Modules/Makefile
                 ])
AC_OUTPUT
