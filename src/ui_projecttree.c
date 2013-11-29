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

#include <log.h>
#include <ui_projecttree.h>

static GtkTreeView *w_treeview;

void ui_projecttree_init(GtkBuilder *builder)
{
	w_treeview = GTK_TREE_VIEW(gtk_builder_get_object(builder, "projecttree"));
}

void ui_projecttree_update(struct task **ts)
{
	struct project **prjs;
	GtkTreeModel *model;
	GtkTreeIter iter;

	log_debug("ui_projecttree_update()");

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(w_treeview));
	gtk_list_store_clear(GTK_LIST_STORE(model));

	prjs = tw_get_projects(ts);
	while (*prjs) {
		gtk_list_store_append(GTK_LIST_STORE(model), &iter);

		gtk_list_store_set(GTK_LIST_STORE(model),
				   &iter,
				   0, (*prjs)->name,
				   1, (*prjs)->count,
				   -1);

		prjs++;
	}
}
