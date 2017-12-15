/*
 * Copyright (C) 2012-2016 jeanfi@gmail.com
 * Copyright (C) 2017, pztrn@pztrn.name
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

#include <string.h>

#include <gtk/gtk.h>

#include <log.h>
#include <tw.h>
#include <ui.h>

static const char *ui_get_priority(GtkComboBox *combo)
{
	int prio;

	prio = gtk_combo_box_get_active(combo);

	switch (prio)
	{
	case 3:
		return "H";
	case 2:
		return "M";
	case 1:
		return "L";
	default:
		return "";
	}
}

static void ui_newtask_populate_project(GtkComboBoxText *w)
{
	struct task **tasks;
	struct project **all_prjs, **prjs;

	tasks = tw_get_all_tasks("pending");

	all_prjs = tw_get_projects(tasks);

	prjs = all_prjs;
	while (*prjs)
	{
		if (strcmp((*prjs)->name, "ALL"))
			gtk_combo_box_text_append_text(w, (*prjs)->name);

		prjs++;
	}

	tw_task_list_free(tasks);
	tw_project_list_free(all_prjs);
}

void ui_newtask_dialog()
{
	// Dialog base.
	GtkWidget *newtask_dialog;
	GtkWidget *newtask_dialog_contentarea;
	GtkWidget *action_box;
	// Task description.
	GtkWidget *desc_label;
	GtkWidget *desc_entry;
	// Project.
	GtkWidget *project_label;
	GtkWidget *project_cb;
	// Priority.
	GtkWidget *priority_label;
	GtkWidget *priority_cb;
	GtkListStore *priority_store;
	GtkTreeIter priority_iter;
	GtkCellRenderer *priority_cb_column;
	// Notes
	GtkWidget *notes_label;
	GtkWidget *notes_scroll;
	GtkEntry *notes_entry;
	GtkTextBuffer *notes_buffer;
	GtkTextIter sIter, eIter;
	// Dialog response.
	gint result;
	// Dialog data from fields.
	const char *desc, *note, *prj, *prio;

	// The dialog.
	newtask_dialog = gtk_dialog_new_with_buttons("Add new task",
												 NULL,
												 GTK_DIALOG_MODAL,
												 "_OK",
												 GTK_RESPONSE_ACCEPT,
												 "_Cancel",
												 GTK_RESPONSE_CANCEL,
												 NULL);

	gtk_window_set_icon_name(GTK_WINDOW(newtask_dialog), "ptasker");
	gtk_window_set_gravity(GTK_WINDOW(newtask_dialog), GDK_GRAVITY_NORTH_WEST);
	gtk_window_set_default_size(GTK_WINDOW(newtask_dialog), 640, 480);

	// Grid for widgets.
	action_box = gtk_grid_new();
	newtask_dialog_contentarea = gtk_dialog_get_content_area(GTK_DIALOG(newtask_dialog));
	gtk_box_set_spacing(GTK_BOX(newtask_dialog_contentarea), 10);
	gtk_grid_set_row_spacing(GTK_GRID(action_box), 5);
	gtk_grid_set_column_spacing(GTK_GRID(action_box), 10);
	gtk_box_pack_start(GTK_BOX(newtask_dialog_contentarea), action_box, FALSE, TRUE, 5);

	// Task description.
	desc_label = gtk_label_new("Description:");
	gtk_grid_attach(GTK_GRID(action_box), desc_label, 0, 0, 1, 1);
	desc_entry = gtk_entry_new();
	gtk_entry_set_placeholder_text(GTK_ENTRY(desc_entry), "Enter task description here...");
	gtk_entry_set_width_chars(GTK_ENTRY(desc_entry), 50);
	gtk_grid_attach(GTK_GRID(action_box), desc_entry, 1, 0, 1, 1);
	gtk_widget_set_hexpand(desc_entry, TRUE);

	// Project.
	project_label = gtk_label_new("Project:");
	gtk_grid_attach(GTK_GRID(action_box), project_label, 0, 1, 1, 1);
	project_cb = gtk_combo_box_text_new_with_entry();
	gtk_grid_attach(GTK_GRID(action_box), project_cb, 1, 1, 1, 1);
	ui_newtask_populate_project(GTK_COMBO_BOX_TEXT(project_cb));

	// Priority.
	priority_store = gtk_list_store_new(1, G_TYPE_STRING);
	gtk_list_store_append(priority_store, &priority_iter);
	gtk_list_store_set(priority_store, &priority_iter, 0, "None", -1);
	gtk_list_store_append(priority_store, &priority_iter);
	gtk_list_store_set(priority_store, &priority_iter, 0, "Low", -1);
	gtk_list_store_append(priority_store, &priority_iter);
	gtk_list_store_set(priority_store, &priority_iter, 0, "Medium", -1);
	gtk_list_store_append(priority_store, &priority_iter);
	gtk_list_store_set(priority_store, &priority_iter, 0, "High", -1);

	priority_label = gtk_label_new("Priority:");
	gtk_grid_attach(GTK_GRID(action_box), priority_label, 0, 2, 1, 1);
	priority_cb = gtk_combo_box_new_with_model(GTK_TREE_MODEL(priority_store));
	g_object_unref(priority_store);
	gtk_grid_attach(GTK_GRID(action_box), priority_cb, 1, 2, 1, 1);

	// ComboBox needs a CellRenderer shit. FFS.
	priority_cb_column = gtk_cell_renderer_text_new();
	gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(priority_cb), priority_cb_column, TRUE);
	gtk_cell_layout_set_attributes(GTK_CELL_LAYOUT(priority_cb), priority_cb_column,
								   "text", 0,
								   NULL);
	gtk_combo_box_set_active(GTK_COMBO_BOX(priority_cb), 0);

	// Task description.
	notes_label = gtk_label_new("Notes:");
	gtk_grid_attach(GTK_GRID(action_box), notes_label, 0, 3, 1, 1);
	notes_scroll = gtk_scrolled_window_new(NULL, NULL);
	gtk_grid_attach(GTK_GRID(action_box), notes_scroll, 1, 3, 1, 1);
	gtk_widget_set_vexpand(notes_scroll, TRUE);
	notes_buffer = gtk_text_buffer_new(NULL);
	notes_entry = gtk_text_view_new_with_buffer(notes_buffer);
	gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(notes_entry), GTK_WRAP_WORD);
	gtk_container_add(GTK_CONTAINER(notes_scroll), notes_entry);

	gtk_widget_show_all(newtask_dialog_contentarea);
	result = gtk_dialog_run(GTK_DIALOG(newtask_dialog));

	if (result == GTK_RESPONSE_ACCEPT)
	{
		log_info("ui_newtask(): ok");
		desc = gtk_entry_get_text(desc_entry);
		prj = gtk_combo_box_text_get_active_text(project_cb);
		prio = ui_get_priority(priority_cb);

		gtk_text_buffer_get_iter_at_offset(notes_buffer, &sIter, 0);
		gtk_text_buffer_get_iter_at_offset(notes_buffer, &eIter, -1);
		note = gtk_text_buffer_get_text(notes_buffer, &sIter, &eIter, TRUE);
		log_info("ui_newtask(): description=%s project=%s priority=%d",
				 desc,
				 prj,
				 prio);

		tw_add(desc, prj, prio, note);

		refresh();
	}
	else
	{
		log_debug("ui_newtask(): cancel");
	}

	gtk_widget_destroy(GTK_WIDGET(newtask_dialog));
}
