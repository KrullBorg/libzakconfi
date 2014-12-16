plugindir = $(libdir)/plugins/db

AM_CPPFLAGS = \
	-I$(top_srcdir) 	\
	$(LIBCONFI_CFLAGS)

plugin_LTLIBRARIES = libdb.la

libdb_la_SOURCES = \
	plgdb.h		\
	plgdb.c

libdb_la_LDFLAGS = $(PLUGIN_LIBTOOL_FLAGS)
libdb_la_LIBADD  = \
	$(top_builddir)/src/libconfi.la		\
	$(LIBCONFI_LIBS)

plugin_DATA = db.plugin

EXTRA_DIST = $(plugin_DATA)
