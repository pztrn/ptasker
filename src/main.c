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
static GtkTreeView *w_treeview;

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
		
		if (i)
			printf("row selected: %d\n", *i);

		task = tasks[*i];

		gtk_tree_path_free(path);

		return task;
	}

	printf("get_selected_task returns NULL\n");

	return NULL;
}

static void refresh()
{
	GtkTreeModel *model;
	struct task **tasks_cur;
	struct task *task;
	int i;
	GtkTreeIter iter;
	/*GtkTreeSelection *sel;*/

	tasks = get_all_tasks();

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

	/*
	sel = gtk_tree_view_get_selection(GTK_TREE_VIEW(w_treeview));
	gtk_tree_model_get_iter_first(model, &iter);
	gtk_tree_selection_select_iter(sel, &iter);*/
}

static int tasksave_clicked_cbk(GtkButton *btn, gpointer data)
{
	struct task *task;
	GtkTextBuffer *buf;
	char *txt, *opts;
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
	txt = escape(ctxt);

	opts = malloc(1
		      + strlen(task->uuid)
		      + strlen(" modify description:\"")
		      + strlen(txt)
		      + strlen("\"")
		      + 1);
	sprintf(opts, " %s modify \"%s\"", task->uuid, txt);
	
	task_exec(opts);

	free(txt);
	
	refresh();

	return FALSE;
}

static int refresh_clicked_cbk(GtkButton *btn, gpointer data)
{
	printf("refresh_clicked_cbk\n");
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

		if (task->note) {
			buf = gtk_text_view_get_buffer(w_note);
			gtk_text_buffer_set_text(buf,
						 task->note,
						 strlen(task->note));
		}

		gtk_entry_set_text(w_description, task->description);
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
	printf("%p\n", window);

	w_treeview = GTK_TREE_VIEW(gtk_builder_get_object(builder, "treeview"));

	w_note = GTK_TEXT_VIEW(gtk_builder_get_object(builder, "tasknote"));

	w_description = GTK_ENTRY(gtk_builder_get_object(builder,
							 "taskdescription"));

	refresh();

	g_signal_connect(w_treeview,
			 "cursor-changed", (GCallback)cursor_changed_cbk, tasks);

	btn = GTK_WIDGET(gtk_builder_get_object(builder, "tasksave"));
	g_signal_connect(btn,
			 "clicked", (GCallback)tasksave_clicked_cbk, tasks);

	btn = GTK_WIDGET(gtk_builder_get_object(builder, "refresh"));
	g_signal_connect(btn,
			 "clicked", (GCallback)refresh_clicked_cbk, tasks);

	g_object_unref(G_OBJECT(builder));

	gtk_widget_show_all(window);

	gtk_main();

	exit(EXIT_SUCCESS);
}
