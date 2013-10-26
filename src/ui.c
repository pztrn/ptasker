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

#include <log.h>
#include <ui.h>

static void save_settings(GtkWindow *window, GSettings *settings)
{
	int w, h, x, y;

	gtk_window_get_size(window, &w, &h);
	gtk_window_get_position(window, &x, &y);

	log_debug("save_settings(): x=%d, y=%d, w=%d, h=%d", x, y, w, h);

	g_settings_set_int(settings, "window-width", w);
	g_settings_set_int(settings, "window-height", h);
	g_settings_set_int(settings, "window-x", x);
	g_settings_set_int(settings, "window-y", y);

	g_settings_sync();
}

static gboolean delete_event_cbk(GtkWidget *w, GdkEvent *evt, gpointer data)
{
	log_debug("delete_event_cbk");

	save_settings(GTK_WINDOW(w), (GSettings *)data);
	gtk_widget_destroy(w);
	gtk_main_quit();

	return TRUE;
}

GtkWindow *create_window(GtkBuilder *builder, GSettings *settings)
{
	GtkWindow *window;
	int x, y, h, w;

	window = GTK_WINDOW(gtk_builder_get_object(builder, "window"));

	x = g_settings_get_int(settings, "window-x");
	y = g_settings_get_int(settings, "window-y");
	w = g_settings_get_int(settings, "window-width");
	h = g_settings_get_int(settings, "window-height");

	gtk_window_set_default_size(window, w, h);
	gtk_window_move(window, x, y);

	g_signal_connect(window, "delete_event",
			 G_CALLBACK(delete_event_cbk), settings);

	return window;
}

