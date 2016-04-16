pluginsdir = $(libdir)/$(PACKAGE)/plugins

AM_CPPFLAGS = \
              -I$(top_srcdir) \
              $(LIBCONFI_CFLAGS)

plugins_LTLIBRARIES = libfile.la

libfile_la_SOURCES = \
                     plgfile.h \
                     plgfile.c

libfile_la_LDFLAGS = $(PLUGIN_LIBTOOL_FLAGS)
libfile_la_LIBADD = \
                    $(top_builddir)/src/libzakconfi.la \
                    $(LIBCONFI_LIBS)

plugins_DATA = file.plugin
