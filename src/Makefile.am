AM_CPPFLAGS = $(WARN_CFLAGS) \
              $(DISABLE_DEPRECATED_CFLAGS) \
              $(LIBCONFI_CFLAGS) \
              -DPLUGINSDIR=\""$(libdir)/$(PACKAGE)/plugins"\" \
              -DG_LOG_DOMAIN=\"ZakConfi\"

LIBS = $(LIBCONFI_LIBS)

lib_LTLIBRARIES = libzakconfi.la

libzakconfi_la_SOURCES = commons.c \
                         confi.c \
                         confipluggable.c

libzakconfi_la_LDFLAGS = -no-undefined

libzakconfi_include_HEADERS = commons.h \
                              libzakconfi.h \
                              confipluggable.h

libzakconfi_includedir = $(includedir)/libzakconfi

install-exec-hook:
	mkdir -p "$(libdir)/$(PACKAGE)/plugins"

uninstall-hook:
	rm -rf "$(libdir)/$(PACKAGE)"

CLEANFILES =

-include $(INTROSPECTION_MAKEFILE)
INTROSPECTION_GIRS =
INTROSPECTION_SCANNER_ARGS = --add-include-path=$(srcdir) --warn-all --accept-unprefixed --identifier-prefix=""
INTROSPECTION_COMPILER_ARGS = --includedir=$(srcdir)
INTROSPECTION_SCANNER_ENV = CC="$(CC)"

if HAVE_INTROSPECTION
introspection_sources = $(libzakconfi_la_SOURCES) $(libzakconfi_include_HEADERS)

ZakConfi-1.0.gir: libzakconfi.la
ZakConfi_1_0_gir_INCLUDES = Gda-5.0 Peas-1.0
ZakConfi_1_0_gir_CFLAGS = $(AM_CPPFLAGS)
ZakConfi_1_0_gir_LIBS = libzakconfi.la
ZakConfi_1_0_gir_FILES = $(introspection_sources)
INTROSPECTION_GIRS += ZakConfi-1.0.gir

girdir = $(datadir)/gir-1.0
gir_DATA = $(INTROSPECTION_GIRS)

typelibdir = $(libdir)/girepository-1.0
typelib_DATA = $(INTROSPECTION_GIRS:.gir=.typelib)

CLEANFILES += $(gir_DATA) $(typelib_DATA)
endif
