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
#include <config.h>

#include <glib/gi18n.h>

#include <log.h>
#include <ui.h>
#include <ui_newtask_diag.h>
#include <ui_projecttree.h>
#include <ui_taskpanel.h>
#include <ui_tasktree.h>

static GtkComboBox *w_status;
static GSettings *gsettings;
static GtkWindow *window;

int newtask_clicked_cbk(GtkButton *btn, gpointer data)
{
	ui_newtask();

	return FALSE;
}

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

	ui_tasktree_save_settings(settings);

	g_settings_sync();
}

int refresh_clicked_cbk(GtkButton *btn, gpointer data)
{
	log_fct_enter();
	refresh();
	log_fct_exit();
	return FALSE;
}


static void ui_quit()
{
	save_settings(window, gsettings);
	gtk_widget_destroy(GTK_WIDGET(window));
	gtk_main_quit();
}

static gboolean delete_event_cbk(GtkWidget *w, GdkEvent *evt, gpointer data)
{
	log_fct_enter();

	ui_quit();

	log_fct_exit();

	return TRUE;
}

static int status_changed_cbk(GtkComboBox *w, gpointer data)
{
	log_fct_enter();

	refresh();

	log_fct_exit();

	return FALSE;
}

GtkWindow *create_window(GtkBuilder *builder, GSettings *settings)
{
	int x, y, w, h;

	gsettings = settings;

	window = GTK_WINDOW(gtk_builder_get_object(builder, "window"));

	w_status = GTK_COMBO_BOX(gtk_builder_get_object(builder, "status"));
	g_signal_connect(w_status,
			 "changed", (GCallback)status_changed_cbk,
			 NULL);

	w = g_settings_get_int(settings, "window-width");
	h = g_settings_get_int(settings, "window-height");
	gtk_window_set_default_size(window, w, h);

	x = g_settings_get_int(settings, "window-x");
	y = g_settings_get_int(settings, "window-y");
	gtk_window_move(window, x, y);

	g_signal_connect(window, "delete_event",
			 G_CALLBACK(delete_event_cbk), settings);

	ui_taskpanel_init(builder);
	ui_tasktree_init(builder);
	ui_projecttree_init(builder);

	ui_tasktree_load_settings(settings);

	return window;
}

const char *ui_get_status_filter()
{
	const char *status;

	log_fct_enter();

	status = gtk_combo_box_get_active_id(w_status);
	log_fct("status: %d", status);

	log_fct_exit();

	return status;
}

void quit_activate_cbk(GtkWidget *menu_item, gpointer data)
{
	log_fct_enter();
	ui_quit();
	log_fct_exit();
}

void about_activate_cbk(GtkWidget *menu_item, gpointer data)
{
	log_fct_enter();

	gtk_show_about_dialog
		(NULL,
		 "comments",
		 _("ptask is a GTK+ task management application"),
		 "copyright",
		 _("Copyright(c) 2010-2013\njeanfi@gmail.com"),
		 "logo-icon-name", "ptask",
		 "program-name", "ptask",
		 "title", _("About ptask"),
		 "version", VERSION,
		 "website", PACKAGE_URL,
		 "website-label", _("ptask Homepage"),
		 NULL);

	log_fct_exit();
}
