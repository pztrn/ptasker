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

struct task {
	int id;
	char *description;
	char *status;
	char *uuid;
	char *note;
	char *project;
};

static struct task **tasks;
static GtkTextView *w_note;
static GtkEntry *w_description;
static GtkTreeView *w_treeview;

static char *task_exec(char *opts)
{
	FILE *f;
	int ret, s;
	char *str, *tmp, *cmd, buf[1024];

	str = NULL;

	cmd = malloc(strlen("task rc.json.array=on ") + strlen(opts) + 1);
	strcpy(cmd, "task rc.json.array=on ");
	strcat(cmd, opts);

	printf("execute: %s\n", cmd);

	f = popen(cmd, "r");

	if (!f) {
		perror("popen");
		goto exit_free;
	}

	str = malloc(1);
	str[0] = '\0';
	while ((s = fread(buf, 1, 1024, f))) {
		tmp = malloc(strlen(str) + s + 1);
		memcpy(tmp, str, strlen(str));
		memcpy(tmp + strlen(str), buf, s);
		tmp[strlen(str) + s] = '\0';
		free(str);
		str = tmp;
	}

	ret = pclose(f);

	if (ret == -1) {
		printf("pclose fails\n");
		perror("pclose");
	}

 exit_free:
	free(cmd);

	return str;
}

static struct json_object *task_exec_json(char *opts)
{
	struct json_object *o;
	char *str;

	str = task_exec(opts);

	if (str) {
		o = json_tokener_parse(str);
		free(str);
		return o;
	}

	return NULL;
}

static struct task **get_all_tasks()
{
	int i, n;
	struct json_object *jtasks, *jtask, *json;
	struct task **tasks;

	jtasks = task_exec_json("export");

	if (!jtasks)
		return NULL;

	n = json_object_array_length(jtasks);

	tasks = malloc((n + 1) * sizeof(struct task *));

	for (i = 0; i < n; i++) {
		jtask = json_object_array_get_idx(jtasks, i);

		tasks[i] = malloc(sizeof(struct task));

		json = json_object_object_get(jtask, "id");
		tasks[i]->id = json_object_get_int(json);

		json = json_object_object_get(jtask, "description");
		tasks[i]->description = strdup(json_object_get_string(json));

		json = json_object_object_get(jtask, "status");
		tasks[i]->status = strdup(json_object_get_string(json));

		json = json_object_object_get(jtask, "project");
		if (json)
			tasks[i]->project
				= strdup(json_object_get_string(json));
		else
			tasks[i]->project = NULL;

		json = json_object_object_get(jtask, "uuid");
		tasks[i]->uuid = strdup(json_object_get_string(json));

		tasks[i]->note = NULL;
	}

	tasks[n] = NULL;

	json_object_put(jtasks);

	return tasks;
}

static struct task *get_selected_task(GtkTreeView *treeview)
{
	GtkTreePath *path;
	GtkTreeViewColumn *cols;
	gint *i;
	struct task *task;

	gtk_tree_view_get_cursor(treeview, &path, &cols);

	if (path) {
		i = gtk_tree_path_get_indices(path);
		
		if (i)
			printf("row selected: %d\n", *i);

		task = tasks[*i];

		gtk_tree_path_free(path);

		return task;
	}

	return NULL;
}

static char *escape(const char *txt)
{
	char *result;
	char *c;

	result = malloc(2*strlen(txt)+1);
	c = result;

	while(*txt) {
		switch(*txt) {
		case '"':
			*c = '\\'; c++;
			*c = '"';
			break;
		case '$':
			*c = '\\'; c++;
			*c = '$';
			break;
		case '&':
			*c = '\\'; c++;
			*c = '&';
			break;
		default:
			*c = *txt;
		}
		c++;
		txt++;
	}

	*c = '\0';

	return result;
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
	GtkTreeIter iter;
	int i;
	GtkTreeModel *model;
	struct task **tasks_cur;
	struct task *task;

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

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(w_treeview));

	tasks = get_all_tasks();

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

	g_signal_connect(w_treeview,
			 "cursor-changed", (GCallback)cursor_changed_cbk, tasks);

	btn = GTK_WIDGET(gtk_builder_get_object(builder, "tasksave"));
	g_signal_connect(btn,
			 "clicked", (GCallback)tasksave_clicked_cbk, tasks);

	g_object_unref(G_OBJECT(builder));

	gtk_widget_show_all(window);

	gtk_main();

	exit(EXIT_SUCCESS);
}
