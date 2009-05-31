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
AC_CHECK_LIB([boost_filesystem-mt], [main], [], [AC_MSG_ERROR([Not Found: boost_filesystem-mt])])
AC_CHECK_LIB([gflags],              [main], [], [AC_MSG_ERROR([Not Found: gflags])])
AC_CHECK_LIB([glog],                [main], [], [AC_MSG_ERROR([Not Found: glog)])
 
# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([fcntl.h stdint.h stdlib.h])
 
# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_CONST
AC_TYPE_SIZE_T
 
# Checks for library functions.
AC_FUNC_MMAP
AC_CHECK_FUNCS([ftruncate getpagesize munmap mremap])
 
AC_OUTPUT
