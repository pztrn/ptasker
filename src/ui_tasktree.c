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
#include <string.h>

#include <gtk/gtk.h>

#include <log.h>
#include <ui_projecttree.h>
#include <ui_tasktree.h>

static GtkTreeView *w_treeview;
static struct task **current_tasks;

enum {
	COL_ID,
	COL_DESCRIPTION,
	COL_PROJECT,
	COL_UUID,
	COL_PRIORITY
};

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

void ui_tasktree_init(GtkBuilder *builder)
{
	GtkTreeModel *model;

	w_treeview = GTK_TREE_VIEW(gtk_builder_get_object(builder, "tasktree"));

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(w_treeview));
	gtk_tree_sortable_set_sort_func(GTK_TREE_SORTABLE(model),
					COL_PRIORITY,
					priority_cmp,
					NULL,
					NULL);
}

void ui_tasktree_load_settings(GSettings *settings)
{
	int sort_col_id;
	GtkSortType sort_order;
	GtkTreeModel *model;

	sort_col_id = g_settings_get_int(settings, "tasks-sort-col");
	sort_order = g_settings_get_int(settings, "tasks-sort-order");
	model = gtk_tree_view_get_model(GTK_TREE_VIEW(w_treeview));
	gtk_tree_sortable_set_sort_column_id(GTK_TREE_SORTABLE(model),
					     sort_col_id, sort_order);
}

void ui_tasktree_save_settings(GSettings *settings)
{
	int sort_col_id;
	GtkTreeModel *model;
	GtkSortType sort_order;

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(w_treeview));
	gtk_tree_sortable_get_sort_column_id(GTK_TREE_SORTABLE(model),
					     &sort_col_id,
					     &sort_order);
	log_debug("ui_tasktree_save_settings(): sort_col_id=%d", sort_col_id);
	log_debug("ui_tasktree_save_settings(): sort_col_order=%d", sort_order);

	g_settings_set_int(settings, "tasks-sort-col", sort_col_id);
	g_settings_set_int(settings, "tasks-sort-order", sort_order);
}

const char *ui_tasktree_get_task_uuid()
{
	struct task *t;

	t = ui_tasktree_get_selected_task();

	if (t)
		return t->uuid;
	else
		return NULL;
}

struct task *ui_tasktree_get_selected_task()
{
	GtkTreePath *path;
	GtkTreeViewColumn *cols;
	struct task **tasks_cur, *result;
	GtkTreeIter iter;
	GtkTreeModel *model;
	GValue value = {0,};
	const char *uuid;

	log_fct_enter();

	result = NULL;

	if (current_tasks) {
		gtk_tree_view_get_cursor(w_treeview, &path, &cols);

		if (path) {
			model = gtk_tree_view_get_model(w_treeview);
			gtk_tree_model_get_iter(model, &iter, path);
			gtk_tree_model_get_value(model,
						 &iter,
						 COL_UUID,
						 &value);

			uuid = g_value_get_string(&value);

			for (tasks_cur = current_tasks; *tasks_cur; tasks_cur++)
				if (!strcmp((*tasks_cur)->uuid, uuid))
					result = *tasks_cur;

			gtk_tree_path_free(path);
		}
	}

	log_fct_exit();

	return result;
}

void ui_tasktree_set_selected_task(const char *uuid)
{
	GtkTreePath *path;
	GtkTreeIter iter;
	GtkTreeModel *model;
	GValue value = {0,};
	const char *c_uuid;

	log_fct_enter();

	if (current_tasks) {
		model = gtk_tree_view_get_model(w_treeview);

		if (!gtk_tree_model_get_iter_first(model, &iter))
			return ;

		path = NULL;
		while (gtk_tree_model_iter_next(model, &iter)) {
			gtk_tree_model_get_value(model,
						 &iter,
						 COL_UUID,
						 &value);
			c_uuid = g_value_get_string(&value);

			if (!strcmp(uuid, c_uuid)) {
				path = gtk_tree_model_get_path(model, &iter);
				break;
			}

			g_value_unset(&value);
		}

		if (!path)
			path = gtk_tree_path_new_first();
		gtk_tree_view_set_cursor(w_treeview, path, NULL, FALSE);
	}

	log_fct_exit();
}


void ui_tasktree_update(struct task **tasks, const char *prj_filter)
{
	GtkTreeModel *model;
	struct task **tasks_cur;
	struct task *task;
	GtkTreeIter iter;
	const char *prj;

	current_tasks = tasks;

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(w_treeview));
	gtk_list_store_clear(GTK_LIST_STORE(model));

	if (current_tasks) {
		for (tasks_cur = current_tasks; *tasks_cur; tasks_cur++) {
			task = (*tasks_cur);

			if (task->project)
				prj = task->project;
			else
				prj = "";

			if (prj_filter && strcmp(prj, prj_filter))
				continue;

			gtk_list_store_append(GTK_LIST_STORE(model), &iter);

			gtk_list_store_set(GTK_LIST_STORE(model),
					   &iter,
					   COL_ID, (*tasks_cur)->id,
					   COL_DESCRIPTION,
					   (*tasks_cur)->description,
					   COL_PROJECT, prj,
					   COL_UUID, (*tasks_cur)->uuid,
					   COL_PRIORITY, (*tasks_cur)->priority,
					   -1);
		}
	}

}

void ui_tasktree_update_filter(const char *prj_filter)
{
	ui_tasktree_update(current_tasks, prj_filter);
}
