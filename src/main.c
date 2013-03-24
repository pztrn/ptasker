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

#include <glib/gi18n.h>
#include <gtk/gtk.h>

#include "note.h"
#include "tw.h"

static struct task **tasks;
static GtkTextView *w_note;
static GtkEntry *w_description;
static GtkEntry *w_project;
static GtkTreeView *w_treeview;
static GtkWidget *w_tasksave_btn;
static GtkWidget *w_taskdone_btn;
static GtkComboBox *w_status;
static GtkComboBox *w_priority;

enum {
	COL_ID,
	COL_DESCRIPTION,
	COL_PROJECT,
	COL_UUID,
	COL_PRIORITY
};

static struct task *get_selected_task(GtkTreeView *treeview)
{
	GtkTreePath *path;
	GtkTreeViewColumn *cols;
	struct task **tasks_cur;
	GtkTreeIter iter;
	GtkTreeModel *model;
	GValue value = {0,};
	const char *uuid;

	printf("get_selected_task\n");

	gtk_tree_view_get_cursor(treeview, &path, &cols);

	if (path) {
		model = gtk_tree_view_get_model(GTK_TREE_VIEW(treeview));
		gtk_tree_model_get_iter(model, &iter, path);
		gtk_tree_model_get_value(model, &iter, COL_UUID, &value);

		uuid = g_value_get_string(&value);

		for (tasks_cur = tasks; *tasks_cur; tasks_cur++)
			if (!strcmp((*tasks_cur)->uuid, uuid))
				return *tasks_cur;

		gtk_tree_path_free(path);
	}

	return NULL;
}

static void clear_task_panel()
{
	GtkTextBuffer *buf;

	gtk_widget_set_sensitive(w_tasksave_btn, 0);
	gtk_widget_set_sensitive(w_taskdone_btn, 0);

	buf = gtk_text_view_get_buffer(w_note);
	gtk_text_buffer_set_text(buf, "", 0);
	gtk_widget_set_sensitive(GTK_WIDGET(w_note), 0);

	gtk_entry_set_text(w_description, "");
	gtk_widget_set_sensitive(GTK_WIDGET(w_description), 0);

	gtk_entry_set_text(w_project, "");
	gtk_widget_set_sensitive(GTK_WIDGET(w_project), 0);

	gtk_combo_box_set_active(w_priority, 0);
	gtk_widget_set_sensitive(GTK_WIDGET(w_priority), 0);
}

static void refresh()
{
	GtkTreeModel *model;
	struct task **tasks_cur;
	struct task *task;
	int i;
	GtkTreeIter iter;
	int status;
	const char *project;

	printf("refresh\n");
	clear_task_panel();

	status = gtk_combo_box_get_active(w_status);
	printf("status: %d\n", status);

	if (tasks)
		tw_task_list_free(tasks);

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
			project = task->project;
		else
			project = "";

		gtk_list_store_set(GTK_LIST_STORE(model),
				   &iter,
				   COL_ID, (*tasks_cur)->id,
				   COL_DESCRIPTION, (*tasks_cur)->description,
				   COL_PROJECT, project,
				   COL_UUID, (*tasks_cur)->uuid,
				   COL_PRIORITY, (*tasks_cur)->priority,
				   -1);
	}
	printf("refresh done\n");
}

int taskdone_clicked_cbk(GtkButton *btn, gpointer data)
{
	struct task *task;

	task = get_selected_task(GTK_TREE_VIEW(w_treeview));
	tw_done(task->uuid);
	refresh();

	return FALSE;
}

static int tasksave_clicked_cbk(GtkButton *btn, gpointer data)
{
	struct task *task;
	GtkTextBuffer *buf;
	char *txt, *pri;
	GtkTextIter sIter, eIter;
	const char *ctxt;
	int priority;

	task = get_selected_task(GTK_TREE_VIEW(w_treeview));

	printf("tasksave_clicked_cbk %d\n", task->id);

	buf = gtk_text_view_get_buffer(w_note);

	gtk_text_buffer_get_iter_at_offset(buf, &sIter, 0);
	gtk_text_buffer_get_iter_at_offset(buf, &eIter, -1);
	txt = gtk_text_buffer_get_text(buf, &sIter, &eIter, TRUE);

	printf("note=%s\n", txt);

	if (!task->note || strcmp(txt, task->note))
		note_put(task->uuid, txt);

	ctxt = gtk_entry_get_text(w_description);
	if (!task->description || strcmp(ctxt, task->description))
		tw_modify_description(task->uuid, ctxt);

	ctxt = gtk_entry_get_text(w_project);
	if (!task->project || strcmp(ctxt, task->project))
		tw_modify_project(task->uuid, ctxt);

	priority = gtk_combo_box_get_active(w_priority);
	printf("priority: %d\n", priority);

	switch (priority) {
	case 3:
		pri = "H";
		break;
	case 2:
		pri = "M";
		break;
	case 1:
		pri = "L";
		break;
	default:
		pri = "";
	}

	if (strcmp(task->priority, pri))
		tw_modify_priority(task->uuid, pri);

	refresh();

	return FALSE;
}

int refresh_clicked_cbk(GtkButton *btn, gpointer data)
{
	printf("refresh_clicked_cbk\n");
	refresh();

	return FALSE;
}

static gboolean delete_event_cbk(GtkWidget *w, GdkEvent *evt, gpointer data)
{
	gtk_widget_destroy(w);
	gtk_main_quit();

	return FALSE;
}


int newtask_clicked_cbk(GtkButton *btn, gpointer data)
{
	gint result;
	static GtkDialog *diag;
	GtkBuilder *builder;
	GtkEntry *entry;
	const char *ctxt;

	printf("newtask_clicked_cbk\n");

	builder = gtk_builder_new();
	gtk_builder_add_from_file
		(builder,
		 PACKAGE_DATA_DIR G_DIR_SEPARATOR_S "ptask.glade",
		 NULL);
	diag = GTK_DIALOG(gtk_builder_get_object(builder, "diag_tasknew"));
	gtk_builder_connect_signals(builder, NULL);

	result = gtk_dialog_run(diag);

	if (result == GTK_RESPONSE_ACCEPT) {
		printf("ok\n");
		entry = GTK_ENTRY(gtk_builder_get_object
				  (builder, "diag_tasknew_description"));
		ctxt = gtk_entry_get_text(entry);

		printf("%s\n", ctxt);

		tw_add(ctxt);
		refresh();
	} else {
		printf("cancel\n");
	}

	g_object_unref(G_OBJECT(builder));

	gtk_widget_destroy(GTK_WIDGET(diag));

	return FALSE;
}

static int status_changed_cbk(GtkComboBox *w, gpointer data)
{
	printf("status_changed_cbk\n");
	refresh();

	return FALSE;
}

static int priority_to_int(const char *str)
{
	switch (*str) {
	case 'H':
		return 3;
	case 'M':
		return 2;
	case 'L':
		return 1;
	default:
		return 0;
	}
}

static int cursor_changed_cbk(GtkTreeView *treeview, gpointer data)
{
	struct task *task;
	GtkTextBuffer *buf;
	int priority;

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
		gtk_widget_set_sensitive(w_taskdone_btn, 1);

		gtk_widget_set_sensitive(GTK_WIDGET(w_priority), 1);
		priority = priority_to_int(task->priority);
		gtk_combo_box_set_active(w_priority, priority);
	} else {
		printf("clear task widgets\n");
		clear_task_panel();
		printf("clear task widgets done\n");
	}

	return FALSE;
}

static gint priority_cmp(GtkTreeModel *model,
			 GtkTreeIter *a,
			 GtkTreeIter *b,
			 gpointer user_data)
{
	GValue v1 = {0,}, v2 = {0,};
	const char *str1, *str2;
	int i1, i2;

	gtk_tree_model_get_value(model, a, COL_PRIORITY, &v1);
	str1 = g_value_get_string(&v1);
	i1 = priority_to_int(str1);

	gtk_tree_model_get_value(model, b, COL_PRIORITY, &v2);
	str2 = g_value_get_string(&v2);
	i2 = priority_to_int(str2);

	if (i1 < i2)
		return -1;
	else if (i1 > i2)
		return 1;
	else
		return 0;
}

int main(int argc, char **argv)
{
	GtkWidget *window, *btn;
	GtkBuilder *builder;
	GtkTreeModel *model;

	setlocale(LC_ALL, "");

#if ENABLE_NLS
	bindtextdomain(PACKAGE, LOCALEDIR);
	textdomain(PACKAGE);
#endif

	gtk_init(NULL, NULL);
	builder = gtk_builder_new();
	gtk_builder_add_from_file
		(builder,
		 PACKAGE_DATA_DIR G_DIR_SEPARATOR_S "ptask.glade",
		 NULL);
	window = GTK_WIDGET(gtk_builder_get_object(builder, "window"));

	g_signal_connect(window, "delete_event",
			 G_CALLBACK(delete_event_cbk), NULL);

	w_treeview = GTK_TREE_VIEW(gtk_builder_get_object(builder, "treeview"));
	model = gtk_tree_view_get_model(GTK_TREE_VIEW(w_treeview));
	gtk_tree_sortable_set_sort_func(GTK_TREE_SORTABLE(model),
					COL_PRIORITY,
					priority_cmp,
					NULL,
					NULL);

	w_note = GTK_TEXT_VIEW(gtk_builder_get_object(builder, "tasknote"));

	w_description = GTK_ENTRY(gtk_builder_get_object(builder,
							 "taskdescription"));
	w_project = GTK_ENTRY(gtk_builder_get_object(builder, "taskproject"));
	w_status = GTK_COMBO_BOX(gtk_builder_get_object(builder, "status"));
	w_priority = GTK_COMBO_BOX(gtk_builder_get_object(builder,
							  "taskpriority"));

	refresh();

	gtk_builder_connect_signals(builder, NULL);

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

	w_taskdone_btn = GTK_WIDGET(gtk_builder_get_object(builder,
							   "taskdone"));
	gtk_widget_set_sensitive(w_taskdone_btn, 0);

	g_object_unref(G_OBJECT(builder));

	gtk_widget_show_all(window);

	gtk_main();

	exit(EXIT_SUCCESS);
}
