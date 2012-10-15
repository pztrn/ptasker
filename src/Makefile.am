AM_LDFLAGS = -Wl,--as-needed 

AM_CPPFLAGS = -Wall -Werror $(GTK_CFLAGS) $(JSON_CFLAGS)

DEFS = -DPACKAGE_DATA_DIR=\"$(pkgdatadir)\" -DLOCALEDIR=\"$(localedir)\" @DEFS@

LIBS = $(JSON_LIBS) $(GTK_LIBS)

bin_PROGRAMS = gtask

gtask_SOURCES = main.c

#dist_man_MANS = gtask.1

#gtask.1: main.c $(top_srcdir)/configure.ac
#	$(MAKE) $(AM_MAKEFLAGS) gtask$(EXEEXT)
#	help2man -N --name="GTk+ Task application" --output=gtask.1 ./gtask$(EXEEXT)

