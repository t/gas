#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)
AC_INIT([graph], [1.0.0], [t@foo.jp])
AC_CONFIG_SRCDIR([src/gas.cpp])
AC_CONFIG_HEADER([config.h])
AC_CONFIG_FILES([Makefile])

AM_INIT_AUTOMAKE

# Checks for programs.
AC_PROG_CXX
AM_CONDITIONAL(GCC, test "$GCC" = yes)   # let the Makefile know if we're gcc

# Uncomment this if you'll be exporting libraries (.so's)
AC_PROG_LIBTOOL
AC_SUBST(LIBTOOL_DEPS)

# Checks for libraries.
# FIXME: Replace `main' with a function in `-lboost_filesystem':
AC_CHECK_LIB([boost_filesystem], [main])
AC_CHECK_LIB([pthread], [main])
AC_CHECK_LIB([gflags], [main])
AC_CHECK_LIB([glog], [main])

# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([fcntl.h stdint.h stdlib.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_CONST
AC_TYPE_SIZE_T

# Checks for library functions.
AC_FUNC_MMAP
AC_CHECK_FUNCS([ftruncate getpagesize munmap])

AC_OUTPUT
