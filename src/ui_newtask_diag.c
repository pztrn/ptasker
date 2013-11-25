/*
 * Copyright (C) 2012-2013 jeanfi@gmail.com
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301 USA
 */

#include <gtk/gtk.h>

#include <log.h>
#include <tw.h>
#include <ui.h>

void ui_newtask()
{
	gint result;
	static GtkDialog *diag;
	GtkBuilder *builder;
	GtkEntry *entry;
	const char *ctxt;

	log_debug("newtask_clicked_cbk");

	builder = gtk_builder_new();
	gtk_builder_add_from_file
		(builder,
		 PACKAGE_DATA_DIR G_DIR_SEPARATOR_S "ptask.glade",
		 NULL);
	diag = GTK_DIALOG(gtk_builder_get_object(builder, "diag_tasknew"));
	gtk_builder_connect_signals(builder, NULL);

	result = gtk_dialog_run(diag);

	if (result == GTK_RESPONSE_ACCEPT) {
		log_debug("ok");
		entry = GTK_ENTRY(gtk_builder_get_object
				  (builder, "diag_tasknew_description"));
		ctxt = gtk_entry_get_text(entry);

		log_debug("%s", ctxt);

		tw_add(ctxt);
		refresh();
	} else {
		log_debug("cancel");
	}

	g_object_unref(G_OBJECT(builder));

	gtk_widget_destroy(GTK_WIDGET(diag));
}
