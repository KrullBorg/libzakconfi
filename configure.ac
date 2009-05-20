#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)
AC_INIT([libconfi], [0.0.2], [azagli@inwind.it])
AC_CONFIG_SRCDIR([src/libconfi.c])
AC_CONFIG_HEADER([config.h])

AM_INIT_AUTOMAKE()
AM_MAINTAINER_MODE()

AC_CANONICAL_SYSTEM

AC_LIBTOOL_WIN32_DLL

# Checks for programs.
AC_PROG_CC
AC_PROG_LIBTOOL
GTK_DOC_CHECK(1.0)

# Checks for libraries.
PKG_CHECK_MODULES(LIBCONFI, [glib-2.0 >= 2.8.0
                             libgdaobj >= 0.0.2])

AC_SUBST(LIBCONFI_CFLAGS)
AC_SUBST(LIBCONFI_LIBS)

# Checks for header files.

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST

# Checks for library functions.

AC_CONFIG_FILES([
  libconfi.pc
  Makefile
  src/Makefile
  tests/Makefile
  data/Makefile
  docs/Makefile
  docs/reference/Makefile
  docs/reference/version.xml
])

AC_OUTPUT
