dnl Process this file with autoconf to produce a configure script.
AC_INIT([tpc], [0.6.3])
AC_CONFIG_SRCDIR([grammar.c])
AM_INIT_AUTOMAKE

dnl Checks for programs.
AC_PROG_AWK
AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PATH_PROG(TPC, tpc, true)

dnl Checks for libraries.

dnl Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS(fcntl.h unistd.h)

dnl Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_TYPE_SIZE_T

dnl Checks for library functions.
AC_FUNC_MEMCMP
AC_CHECK_FUNCS(strdup)

AM_CONFIG_HEADER(config.h)
AC_OUTPUT(Makefile)
