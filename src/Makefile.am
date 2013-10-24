SUBDIRS=glade

# -export-dynamic is need for defining handlers in the glade file.
AM_LDFLAGS = -Wl,--as-needed  -export-dynamic

AM_CPPFLAGS = -Wall -Werror $(GTK_CFLAGS) $(JSON_CFLAGS)

DEFS = -DPACKAGE_DATA_DIR=\"$(pkgdatadir)\" -DLOCALEDIR=\"$(localedir)\" @DEFS@

LIBS = $(JSON_LIBS) $(GTK_LIBS)

bin_PROGRAMS = ptask

ptask_SOURCES = main.c \
	log.c log.h \
	note.c note.h \
	pstr.c pstr.h \
	ptime.c ptime.h \
	tw.c tw.h \
	ui.c ui.h

dist_man_MANS = ptask.1

ptask.1: main.c $(top_srcdir)/configure.ac
	$(MAKE) $(AM_MAKEFLAGS) ptask$(EXEEXT)
	help2man -N --name="GTk+ Task application" --output=ptask.1 ./ptask$(EXEEXT)

gsettings_SCHEMAS=ptask.gschema.xml
EXTRA_DIST=$(gsettings_SCHEMAS)

@GSETTINGS_RULES@
