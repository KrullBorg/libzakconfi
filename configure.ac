#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)
AC_INIT([libzakconfi], [0.10.0], [azagli@libero.it])
AC_CONFIG_SRCDIR([src/confi.c])
AC_CONFIG_HEADER([config.h])

AM_INIT_AUTOMAKE(-Wall)

m4_ifdef([AM_SILENT_RULES],[AM_SILENT_RULES([yes])])

AM_MAINTAINER_MODE()

AC_CANONICAL_SYSTEM

AC_LIBTOOL_WIN32_DLL

# Checks for programs.
AC_PROG_CC
AC_PROG_LIBTOOL
GTK_DOC_CHECK(1.0)

GOBJECT_INTROSPECTION_CHECK([1.30.0])

# Checks for libraries.
PKG_CHECK_MODULES(LIBCONFI, [libgdaex >= 0.5.0
                             libpeas-1.0 >= 1.12.0])

AC_SUBST(LIBCONFI_CFLAGS)
AC_SUBST(LIBCONFI_LIBS)

# Checks for header files.

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST

# Checks for library functions.

PLUGIN_LIBTOOL_FLAGS="-module -avoid-version -no-undefined"
AC_SUBST(PLUGIN_LIBTOOL_FLAGS)

dnl ******************************
dnl Check for Operating System
dnl ******************************

platform_win32=no

case "$host" in
*-mingw*)
    platform_win32=yes
    ;;
esac

AM_CONDITIONAL(PLATFORM_WIN32, [test $platform_win32 = yes])

AC_CONFIG_FILES([
  libzakconfi.pc
  Makefile
  src/Makefile
  plugins/Makefile
  plugins/db/Makefile
  plugins/file/Makefile
  tests/Makefile
  tools/Makefile
  data/Makefile
  docs/Makefile
  docs/reference/Makefile
  docs/reference/version.xml
])

AC_OUTPUT
