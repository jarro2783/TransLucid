AC_DEFUN([TL_CHECK_ICU_HELPER], [

  LINK_METHOD=$2

  AC_MSG_NOTICE([checking that ICU is linkable ${LINK_METHOD}ally])
  ICU_FOUND=no

  AC_CHECK_LIB([icuuc], [u_isprint], ICU_FOUND=yes,
  ,
  [$1])

  if test "x$ICU_FOUND" = "xno"; then
    AC_CHECK_LIB([icuuc], [u_isprint_54], ICU_FOUND=yes,
    ,
    [$1])
  fi

  if test "x$ICU_FOUND" = "xno"; then
    AC_CHECK_LIB([icuuc], [u_isprint_53], ICU_FOUND=yes,
    ,
    [$1])
  fi

  if test "x$ICU_FOUND" = "xno"; then
    AC_CHECK_LIB([icuuc], [u_isprint_52], ICU_FOUND=yes,
    ,
    [$1])
  fi

  if test "x$ICU_FOUND" = "xno"; then
    AC_CHECK_LIB([icuuc], [u_isprint_51], ICU_FOUND=yes,
    ,
    [$1])
  fi

  if test "x$ICU_FOUND" = "xno"; then
    AC_CHECK_LIB([icuuc], [u_isprint_50], ICU_FOUND=yes,
    ,
    [$1])
  fi

  if test "x$ICU_FOUND" = "xno"; then
    AC_CHECK_LIB([icuuc], [u_isprint_49], ICU_FOUND=yes,
    ,
    [$1])
  fi

  if test "x$ICU_FOUND" = "xno"; then
    AC_CHECK_LIB([icuuc], [u_isprint_48], ICU_FOUND=yes,
    ,
    [$1])
  fi

  if test "x$ICU_FOUND" = "xno"; then
    AC_CHECK_LIB([icuuc], [u_isprint_47], ICU_FOUND=yes,
    ,
    [$1])
  fi

  if test "x$ICU_FOUND" = "xno"; then
    AC_CHECK_LIB([icuuc], [u_isprint_46], ICU_FOUND=yes,
    ,
    [$1])
  fi

  if test "x$ICU_FOUND" = "xno"; then
    AC_MSG_FAILURE([Cannot find the ICU ${LINK_METHOD} libraries])
  fi

])



AC_DEFUN([TL_CHECK_ICU],
[
  TL_CHECK_ICU_HELPER([${ICU_LIBS}], [dynamic])
  if test "x$enable_static" = "xyes"; then
    TL_CHECK_ICU_HELPER([${ICU_LIBS} -static], [static])
  fi
])
