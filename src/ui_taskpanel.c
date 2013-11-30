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

#include <log.h>
#include <ui_taskpanel.h>

static GtkTextView *w_note;
static GtkEntry *w_description;
static GtkEntry *w_project;
static GtkComboBox *w_priority;
static GtkButton *w_tasksave_btn;
static GtkButton *w_taskdone_btn;
static GtkButton *w_taskcancel_btn;

static void enable(int enable)
{
	GtkTextBuffer *buf;

	gtk_widget_set_sensitive(GTK_WIDGET(w_tasksave_btn), enable);
	gtk_widget_set_sensitive(GTK_WIDGET(w_taskdone_btn), enable);
	gtk_widget_set_sensitive(GTK_WIDGET(w_taskcancel_btn), enable);

	buf = gtk_text_view_get_buffer(w_note);
	if (!enable)
		gtk_text_buffer_set_text(buf, "", 0);
	gtk_widget_set_sensitive(GTK_WIDGET(w_note), enable);

	if (!enable)
		gtk_entry_set_text(w_description, "");
	gtk_widget_set_sensitive(GTK_WIDGET(w_description), enable);

	if (!enable)
		gtk_entry_set_text(w_project, "");
	gtk_widget_set_sensitive(GTK_WIDGET(w_project), enable);

	if (!enable)
		gtk_combo_box_set_active(w_priority, 0);
	gtk_widget_set_sensitive(GTK_WIDGET(w_priority), enable);
}

void ui_taskpanel_init(GtkBuilder *builder)
{
	log_fct(__func__, "ENTER");

	w_note = GTK_TEXT_VIEW(gtk_builder_get_object(builder, "tasknote"));

	w_description = GTK_ENTRY(gtk_builder_get_object(builder,
							 "taskdescription"));
	w_project = GTK_ENTRY(gtk_builder_get_object(builder, "taskproject"));
	w_priority = GTK_COMBO_BOX(gtk_builder_get_object(builder,
							  "taskpriority"));

	w_tasksave_btn = GTK_BUTTON(gtk_builder_get_object(builder, 
							   "tasksave"));
	w_taskdone_btn = GTK_BUTTON(gtk_builder_get_object(builder, 
							   "taskdone"));
	w_taskcancel_btn = GTK_BUTTON(gtk_builder_get_object(builder, 
							     "taskcancel"));

	enable(0);

	log_fct(__func__, "EXIT");
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

void ui_taskpanel_update(struct task *task)
{
	GtkTextBuffer *buf;
	int priority;

	if (task) {
		buf = gtk_text_view_get_buffer(w_note);
		if (task->note)
			gtk_text_buffer_set_text(buf,
						 task->note,
						 strlen(task->note));
		else
			gtk_text_buffer_set_text(buf, "", 0);

		gtk_entry_set_text(w_description, task->description);

		if (task->project)
			gtk_entry_set_text(w_project, task->project);
		else
			gtk_entry_set_text(w_project, "");

		priority = priority_to_int(task->priority);
		gtk_combo_box_set_active(w_priority, priority);
		
		enable(1);
	} else {
		enable(0);
	}
}

