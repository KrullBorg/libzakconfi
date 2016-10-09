AM_CPPFLAGS = $(WARN_CFLAGS) \
              $(DISABLE_DEPRECATED_CFLAGS) \
              $(LIBCONFI_CFLAGS) \
              -I$(top_srcdir)/src \
              -DPLUGINSDIR=\""$(libdir)/$(PACKAGE)/plugins"\"

LIBS = $(LIBCONFI_LIBS) \
       -L../src -lzakconfi \
       -export-dynamic

LDADD = $(top_builddir)/src/libzakconfi.la

noinst_PROGRAMS = test \
                  test_add_config \
                  test_add_config_from_confi \
                  test_get_configs_list \
                  test_get_tree \
                  test_set_config

EXTRA_DIST = gir.py
