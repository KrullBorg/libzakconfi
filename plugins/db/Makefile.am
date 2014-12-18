pluginsdir = $(libdir)/$(PACKAGE)/plugins

AM_CPPFLAGS = \
	-I$(top_srcdir) \
	$(LIBCONFI_CFLAGS)

plugins_LTLIBRARIES = libdb.la

libdb_la_SOURCES = \
	plgdb.h \
	plgdb.c

libdb_la_LDFLAGS = $(PLUGIN_LIBTOOL_FLAGS)
libdb_la_LIBADD = \
	$(top_builddir)/src/libconfi.la \
	$(LIBCONFI_LIBS)

plugins_DATA = db.plugin
