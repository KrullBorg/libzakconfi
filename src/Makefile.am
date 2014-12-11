AM_CPPFLAGS = $(WARN_CFLAGS) \
              $(DISABLE_DEPRECATED_CFLAGS) \
              $(LIBCONFI_CFLAGS)

LIBS = $(LIBCONFI_LIBS)

lib_LTLIBRARIES = libconfi.la

libconfi_la_SOURCES = libconfi.c \
                      confipluggable.c

libconfi_la_LDFLAGS = -no-undefined

include_HEADERS = libconfi.h \
                  confipluggable.h

libconfi_includedir = $(includedir)/libconfi

CLEANFILES =

-include $(INTROSPECTION_MAKEFILE)
INTROSPECTION_GIRS =
INTROSPECTION_SCANNER_ARGS = --add-include-path=$(srcdir) --warn-all --accept-unprefixed --identifier-prefix=""
INTROSPECTION_COMPILER_ARGS = --includedir=$(srcdir)
INTROSPECTION_SCANNER_ENV = CC="$(CC)"

if HAVE_INTROSPECTION
introspection_sources = $(libconfi_la_SOURCES) $(include_HEADERS)

Confi-1.0.gir: libconfi.la
Confi_1_0_gir_INCLUDES = Gda-5.0
Confi_1_0_gir_CFLAGS = $(AM_CPPFLAGS)
Confi_1_0_gir_LIBS = libconfi.la
Confi_1_0_gir_FILES = $(introspection_sources)
INTROSPECTION_GIRS += Confi-1.0.gir

girdir = $(datadir)/gir-1.0
gir_DATA = $(INTROSPECTION_GIRS)

typelibdir = $(libdir)/girepository-1.0
typelib_DATA = $(INTROSPECTION_GIRS:.gir=.typelib)

CLEANFILES += $(gir_DATA) $(typelib_DATA)
endif