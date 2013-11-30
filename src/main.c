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
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <sys/stat.h>

#include <json/json.h>

#include <glib/gi18n.h>

#include <config.h>

#include <log.h>
#include <note.h>
#include <tw.h>
#include <ui.h>
#include <ui_projecttree.h>
#include <ui_taskpanel.h>
#include <ui_tasktree.h>

static const char *program_name;
static struct task **tasks;
static GtkTreeView *w_treeview;
static GtkComboBox *w_status;
static GSettings *settings;

enum {
	COL_ID,
	COL_DESCRIPTION,
	COL_PROJECT,
	COL_UUID,
	COL_PRIORITY
};

static struct option long_options[] = {
	{"version", no_argument, 0, 'v'},
	{"help", no_argument, 0, 'h'},
	{"debug", required_argument, 0, 'd'},
	{0, 0, 0, 0}
};

static void print_version()
{
	printf("ptask %s\n", VERSION);
	printf(_("Copyright (C) %s jeanfi@gmail.com\n"
		 "License GPLv2: GNU GPL version 2 or later "
		 "<http://www.gnu.org/licenses/old-licenses/gpl-2.0.html>\n"
		 "This is free software: you are free to change and "
		 " redistribute it.\n"
		 "There is NO WARRANTY, to the extent permitted by law.\n"),
	       "2012-2013");
}

static void print_help()
{
	printf(_("Usage: %s [OPTION]...\n"), program_name);

	puts(_("ptask is a task management user interface based"
	       " on taskwarrior."));

	puts("");
	puts(_("Options:"));
	puts(_("  -h, --help          display this help and exit\n"
	       "  -v, --version       display version information and exit"));

	puts("");

	puts(_("  -d, --debug=LEVEL   "
	       "set the debug level, integer between 0 and 3"));

	puts("");

	printf(_("Report bugs to: %s\n"), PACKAGE_BUGREPORT);
	puts("");
	printf(_("%s home page: <%s>\n"), PACKAGE_NAME, PACKAGE_URL);
}

static struct task *get_selected_task(GtkTreeView *treeview)
{
	GtkTreePath *path;
	GtkTreeViewColumn *cols;
	struct task **tasks_cur;
	GtkTreeIter iter;
	GtkTreeModel *model;
	GValue value = {0,};
	const char *uuid;

	log_debug("get_selected_task");

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

void refresh()
{
	GtkWidget *dialog;
	GtkTreeModel *model;
	struct task **tasks_cur;
	struct task *task;
	int i;
	GtkTreeIter iter;
	const char *project;

	log_fct_enter();
	ui_taskpanel_update(NULL);

	if (tasks)
		tw_task_list_free(tasks);

	tasks = tw_get_all_tasks(ui_get_status_filter());

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(w_treeview));
	gtk_list_store_clear(GTK_LIST_STORE(model));

	if (tasks) {
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
					   COL_DESCRIPTION,
					   (*tasks_cur)->description,
					   COL_PROJECT, project,
					   COL_UUID, (*tasks_cur)->uuid,
					   COL_PRIORITY, (*tasks_cur)->priority,
					   -1);
		}
		ui_projecttree_update(tasks);
	} else {
		dialog = gtk_message_dialog_new(NULL,
						GTK_DIALOG_DESTROY_WITH_PARENT,
						GTK_MESSAGE_ERROR,
						GTK_BUTTONS_CLOSE,
						_("Error loading tasks, verify "
						  "that a supported version of "
						  "taskwarrior is installed "));
		gtk_dialog_run(GTK_DIALOG(dialog));
		gtk_widget_destroy(dialog);
	}
	log_fct(__func__, "EXIT");
}

int refresh_clicked_cbk(GtkButton *btn, gpointer data)
{
	log_debug("refresh_clicked_cbk");
	refresh();

	return FALSE;
}

static int status_changed_cbk(GtkComboBox *w, gpointer data)
{
	log_debug("status_changed_cbk");
	refresh();

	return FALSE;
}

static int cursor_changed_cbk(GtkTreeView *treeview, gpointer data)
{
	log_fct_enter();

	ui_taskpanel_update(get_selected_task(treeview));

	log_fct_exit();

	return FALSE;
}

static void log_init()
{
	char *home, *path, *dir;

	home = getenv("HOME");

	if (!home)
		return ;

	dir = malloc(strlen(home)+1+strlen(".ptask")+1);
	sprintf(dir, "%s/%s", home, ".ptask");
	mkdir(dir, 0777);

	path = malloc(strlen(dir)+1+strlen("log")+1);
	sprintf(path, "%s/%s", dir, "log");

	log_open(path);

	free(dir);
	free(path);
}

int main(int argc, char **argv)
{
	GtkWindow *window;
	GtkBuilder *builder;
	int optc, cmdok, opti;

	program_name = argv[0];

	setlocale(LC_ALL, "");

#if ENABLE_NLS
	bindtextdomain(PACKAGE, LOCALEDIR);
	textdomain(PACKAGE);
#endif

	cmdok = 1;
	while ((optc = getopt_long(argc, argv, "vhd:", long_options,
				   &opti)) != -1) {
		switch (optc) {
		case 'h':
			print_help();
			exit(EXIT_SUCCESS);
		case 'v':
			print_version();
			exit(EXIT_SUCCESS);
		case 'd':
			log_level = atoi(optarg);
			log_info(_("Enables debug mode."));
			break;
		default:
			cmdok = 0;
			break;
		}
	}

	if (!cmdok || optind != argc) {
		fprintf(stderr, _("Try `%s --help' for more information.\n"),
			program_name);
		exit(EXIT_FAILURE);
	}

	log_init();

	gtk_init(NULL, NULL);

	settings = g_settings_new("ptask");

	builder = gtk_builder_new();
	gtk_builder_add_from_file
		(builder,
		 PACKAGE_DATA_DIR G_DIR_SEPARATOR_S "ptask.glade",
		 NULL);
	window = create_window(builder, settings);

	ui_taskpanel_init(builder);
	ui_tasktree_init(builder);
	ui_projecttree_init(builder);

	w_treeview = GTK_TREE_VIEW(gtk_builder_get_object(builder, "tasktree"));

	w_status = GTK_COMBO_BOX(gtk_builder_get_object(builder, "status"));

	gtk_builder_connect_signals(builder, NULL);

	g_signal_connect(w_treeview,
			 "cursor-changed", (GCallback)cursor_changed_cbk,
			 tasks);
	g_signal_connect(w_status,
			 "changed", (GCallback)status_changed_cbk,
			 tasks);

	g_object_unref(G_OBJECT(builder));

	refresh();

	gtk_widget_show_all(GTK_WIDGET(window));

	gtk_main();

	exit(EXIT_SUCCESS);
}
