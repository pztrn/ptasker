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
#include <stdio.h>
#include <string.h>

#include <log.h>
#include <ui_projecttree.h>

enum {
	COL_NAME,
	COL_COUNT
};

static GtkTreeView *w_treeview;

void ui_projecttree_init(GtkBuilder *builder)
{
	w_treeview = GTK_TREE_VIEW(gtk_builder_get_object(builder,
							  "projecttree"));
}

const char *ui_projecttree_get_project()
{
	GtkTreePath *path;
	GtkTreeViewColumn *cols;
	GtkTreeIter iter;
	GtkTreeModel *model;
	GValue value = {0,};
	const char *prj;

	log_fct_enter();

	gtk_tree_view_get_cursor(w_treeview, &path, &cols);

	if (path) {
		model = gtk_tree_view_get_model(GTK_TREE_VIEW(w_treeview));
		gtk_tree_model_get_iter(model, &iter, path);
		gtk_tree_model_get_value(model, &iter, COL_NAME, &value);

		prj = g_value_get_string(&value);
	} else {
		prj = NULL;
	}

	log_fct_exit();

	return prj;
}


void ui_projecttree_update(struct task **ts)
{
	struct project **prjs, **cur;
	GtkTreeModel *model;
	GtkTreeIter iter;

	log_fct_enter();

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(w_treeview));
	gtk_list_store_clear(GTK_LIST_STORE(model));

	prjs = tw_get_projects(ts);
	for (cur = prjs; *cur; cur++) {
		gtk_list_store_append(GTK_LIST_STORE(model), &iter);

		gtk_list_store_set(GTK_LIST_STORE(model),
				   &iter,
				   COL_NAME, (*cur)->name,
				   COL_COUNT, (*cur)->count,
				   -1);
	}

	tw_project_list_free(prjs);

	log_fct_exit();
}

