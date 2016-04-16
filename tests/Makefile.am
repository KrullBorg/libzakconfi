AM_CPPFLAGS = $(WARN_CFLAGS) \
              $(DISABLE_DEPRECATED_CFLAGS) \
              $(LIBCONFI_CFLAGS) \
              -I$(top_srcdir)/src

LIBS = $(LIBCONFI_LIBS) \
       -L../src -lzakconfi \
       -export-dynamic

LDADD = $(top_builddir)/src/libzakconfi.la

noinst_PROGRAMS = test

EXTRA_DIST = gir.py
