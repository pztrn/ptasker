SUBDIRS=glade

# -export-dynamic is need for defining handlers in the glade file.
AM_LDFLAGS = -Wl,--as-needed  -export-dynamic

AM_CPPFLAGS = -Wall $(GTK_CFLAGS) $(JSON_CFLAGS)

DEFS = -DPACKAGE_DATA_DIR=\"$(pkgdatadir)\" -DLOCALEDIR=\"$(localedir)\" @DEFS@

LIBS = $(JSON_LIBS) $(GTK_LIBS)

bin_PROGRAMS = ptasker

ptasker_SOURCES = log.c log.h \
	main.c \
	note.c note.h \
	pio.c pio.h \
	pstr.c pstr.h \
	ptime.c ptime.h \
	settings.c settings.h \
	tw.c tw.h \
	ui.c ui.h \
	ui_newtask_diag.c ui_newtask_diag.h \
	ui_projecttree.c ui_projecttree.h \
	ui_taskpanel.c ui_taskpanel.h \
	ui_tasktree.c ui_tasktree.h \
	ui_toolbar.c ui_toolbar.h

dist_man_MANS = ptasker.1

ptasker.1: main.c $(top_srcdir)/configure.ac
	$(MAKE) $(AM_MAKEFLAGS) ptasker$(EXEEXT)
	help2man -N --name="GTk+ Task application" --output=ptasker.1 ./ptasker$(EXEEXT)

gsettings_SCHEMAS=ptasker.gschema.xml
EXTRA_DIST=$(gsettings_SCHEMAS)

@GSETTINGS_RULES@
