/*
 * Copyright (C) 2010-2012 jeanfi@gmail.com
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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <json/json.h>

#include <gtk/gtk.h>

#include "tw.h"

static struct task **tasks;
static GtkTextView *w_note;
static GtkEntry *w_description;
static GtkEntry *w_project;
static GtkTreeView *w_treeview;
static GtkWidget *w_tasksave_btn;
static GtkComboBox *w_status;

static struct task *get_selected_task(GtkTreeView *treeview)
{
	GtkTreePath *path;
	GtkTreeViewColumn *cols;
	gint *i;
	struct task *task;

	printf("get_selected_task\n");

	gtk_tree_view_get_cursor(treeview, &path, &cols);

	if (path) {
		i = gtk_tree_path_get_indices(path);

		if (i) {
			printf("row selected: %d\n", *i);

			task = tasks[*i];
		} else {
			task = NULL;
		}

		gtk_tree_path_free(path);
	} else {
		task = NULL;
	}

	return task;
}

static void clear_task_panel()
{
	GtkTextBuffer *buf;

	gtk_widget_set_sensitive(w_tasksave_btn, 0);

	buf = gtk_text_view_get_buffer(w_note);
	gtk_text_buffer_set_text(buf, "", 0);
	gtk_widget_set_sensitive(GTK_WIDGET(w_note), 0);

	gtk_entry_set_text(w_description, "");
	gtk_widget_set_sensitive(GTK_WIDGET(w_description), 0);

	gtk_entry_set_text(w_project, "");
	gtk_widget_set_sensitive(GTK_WIDGET(w_project), 0);
}

static void refresh()
{
	GtkTreeModel *model;
	struct task **tasks_cur;
	struct task *task;
	int i;
	GtkTreeIter iter;
	int status;

	clear_task_panel();

	status = gtk_combo_box_get_active(w_status);
	printf("status: %d\n", status);

	switch (status) {
	case 0:
		tasks = tw_get_all_tasks("pending");
		break;
	case 1:
		tasks = tw_get_all_tasks("completed");
		break;
	default:
		tasks = tw_get_all_tasks("pending");
	}

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(w_treeview));
	gtk_list_store_clear(GTK_LIST_STORE(model));
	for (tasks_cur = tasks, i = 0; *tasks_cur; tasks_cur++, i++) {
		task = (*tasks_cur);

		gtk_list_store_append(GTK_LIST_STORE(model), &iter);

		if (task->project)
			gtk_list_store_set(GTK_LIST_STORE(model),
					   &iter,
					   2, task->project,
					   -1);

		gtk_list_store_set(GTK_LIST_STORE(model),
				   &iter,
				   0, (*tasks_cur)->id,
				   1, (*tasks_cur)->description,
				   -1);
	}
}

static int tasksave_clicked_cbk(GtkButton *btn, gpointer data)
{
	struct task *task;
	GtkTextBuffer *buf;
	char *txt;
	GtkTextIter sIter, eIter;
	const char *ctxt;

	task = get_selected_task(GTK_TREE_VIEW(w_treeview));

	printf("tasksave_clicked_cbk %d\n", task->id);

	if (task->note) {
		buf = gtk_text_view_get_buffer(w_note);

		gtk_text_buffer_get_iter_at_offset(buf, &sIter, 0);
		gtk_text_buffer_get_iter_at_offset(buf, &eIter, -1);
		txt = gtk_text_buffer_get_text(buf, &sIter, &eIter, TRUE);

		txt = escape(txt);

		printf("%s\n", txt);
	}

	ctxt = gtk_entry_get_text(w_description);
	if (!task->description || strcmp(ctxt, task->description))
		tw_modify_description(task->uuid, ctxt);

	ctxt = gtk_entry_get_text(w_project);
	if (!task->project || strcmp(ctxt, task->project))
		tw_modify_project(task->uuid, ctxt);

	refresh();

	return FALSE;
}

static int refresh_clicked_cbk(GtkButton *btn, gpointer data)
{
	printf("refresh_clicked_cbk\n");
	refresh();

	return FALSE;
}

static int status_changed_cbk(GtkComboBox *w, gpointer data)
{
	printf("status_changed_cbk\n");
	refresh();

	return FALSE;
}

static int cursor_changed_cbk(GtkTreeView *treeview, gpointer data)
{
	struct task *task;
	GtkTextBuffer *buf;

	printf("cursor_changed_cbk\n");

	task = get_selected_task(treeview);

	if (task) {

		buf = gtk_text_view_get_buffer(w_note);
		if (task->note)
			gtk_text_buffer_set_text(buf,
						 task->note,
						 strlen(task->note));
		else
			gtk_text_buffer_set_text(buf, "", 0);
		gtk_widget_set_sensitive(GTK_WIDGET(w_note), 1);

		gtk_entry_set_text(w_description, task->description);
		gtk_widget_set_sensitive(GTK_WIDGET(w_description), 1);

		if (task->project)
			gtk_entry_set_text(w_project, task->project);
		else
			gtk_entry_set_text(w_project, "");
		gtk_widget_set_sensitive(GTK_WIDGET(w_project), 1);

		gtk_widget_set_sensitive(w_tasksave_btn, 1);
	} else {
		printf("clear task widgets\n");
		clear_task_panel();
	}

	return FALSE;
}

int main(int argc, char **argv)
{
	GtkWidget *window;
	GtkWidget *btn;
	GtkBuilder *builder;

	gtk_init(NULL, NULL);
	builder = gtk_builder_new();
	gtk_builder_add_from_file
		(builder,
		 PACKAGE_DATA_DIR G_DIR_SEPARATOR_S "gtask.glade",
		 NULL);
	window = GTK_WIDGET(gtk_builder_get_object(builder, "window"));

	w_treeview = GTK_TREE_VIEW(gtk_builder_get_object(builder, "treeview"));

	w_note = GTK_TEXT_VIEW(gtk_builder_get_object(builder, "tasknote"));

	w_description = GTK_ENTRY(gtk_builder_get_object(builder,
							 "taskdescription"));
	w_project = GTK_ENTRY(gtk_builder_get_object(builder, "taskproject"));
	w_status = GTK_COMBO_BOX(gtk_builder_get_object(builder, "status"));

	refresh();

	g_signal_connect(w_treeview,
			 "cursor-changed", (GCallback)cursor_changed_cbk,
			 tasks);
	g_signal_connect(w_status,
			 "changed", (GCallback)status_changed_cbk,
			 tasks);
	btn = GTK_WIDGET(gtk_builder_get_object(builder, "tasksave"));
	g_signal_connect(btn,
			 "clicked", (GCallback)tasksave_clicked_cbk, tasks);
	gtk_widget_set_sensitive(btn, 0);
	w_tasksave_btn = btn;

	btn = GTK_WIDGET(gtk_builder_get_object(builder, "refresh"));
	g_signal_connect(btn,
			 "clicked", (GCallback)refresh_clicked_cbk, tasks);

	g_object_unref(G_OBJECT(builder));

	gtk_widget_show_all(window);

	gtk_main();

	exit(EXIT_SUCCESS);
}
