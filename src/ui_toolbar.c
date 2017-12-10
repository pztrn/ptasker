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

#include "ui_toolbar.h"

#include <gtk/gtk.h>
#include "log.h"
#include "tw.h"
#include "ui.h"
#include "ui_newtask_diag.h"
#include "ui_taskpanel.h"

int taskcancel_clicked_cbk(GtkButton *btn, gpointer data)
{
    log_fct_enter();

    ui_taskpanel_update(NULL);

    log_fct_exit();

    return FALSE;
}

// Callback for new task button click.
int newtask_clicked_cbk(GtkButton *btn, gpointer data)
{
    ui_newtask();

    return FALSE;
}

int refresh_clicked_cbk(GtkButton *btn, gpointer data)
{
    log_fct_enter();
    refresh();
    log_fct_exit();
    return FALSE;
}

int taskdone_clicked_cbk(GtkButton *btn, gpointer data)
{
    struct task *current_task;
    current_task = (struct task *)taskpanel_get_current_task();

    log_fct_enter();

    if (current_task != NULL)
    {
        log_fct("uuid=%d", current_task->uuid);
        tw_task_done(current_task->uuid);
        refresh();
    }

    log_fct_exit();

    return FALSE;
}

int taskremove_clicked_cbk(GtkButton *btn, gpointer data)
{
    struct task *current_task;
    current_task = (struct task *)taskpanel_get_current_task();

    log_fct_enter();

    if (current_task)
    {
        log_fct("uuid=%d", current_task->uuid);
        tw_task_remove(current_task->uuid);
        refresh();
    }

    log_fct_exit();

    return FALSE;
}

static int tasksave_clicked_cbk(GtkButton *btn, gpointer data)
{
    taskpanel_save();

    refresh();

    return FALSE;
}

// Toolbar initialization.
// As of bugs in Glade about inability to add something else than just
// a button, toolbar initialization will be done manually.
void ui_toolbar_init(GtkBuilder *builder)
{
    GtkToolbar *w_toolbar;
    // New task button and icon.
    GtkToolItem *ti_newtask;
    GtkWidget *ti_newtask_icon;
    // Refresh tasks button and icon.
    GtkToolItem *ti_refresh;
    GtkWidget *ti_refresh_icon;
    // Separator.
    GtkToolItem *ti_sep1;
    // Save task.
    GtkToolItem *ti_savetask;
    GtkWidget *ti_savetask_icon;
    // Cancel task.
    GtkToolItem *ti_canceltask;
    GtkWidget *ti_canceltask_icon;
    // Mark task as done.
    GtkToolItem *ti_taskdone;
    GtkWidget *ti_taskdone_icon;
    // Remove task button.
    GtkToolItem *ti_removetask;
    GtkWidget *ti_removetask_icon;

    w_toolbar = GTK_TOOLBAR(gtk_builder_get_object(builder, "maintoolbar"));

    // New task button.
    ti_newtask_icon = gtk_image_new_from_icon_name("gtk-new", GTK_ICON_SIZE_SMALL_TOOLBAR);
    ti_newtask = gtk_tool_button_new(ti_newtask_icon, "New task");
    gtk_tool_item_set_tooltip_text(ti_newtask, "Create new task");
    g_signal_connect(ti_newtask, "clicked", G_CALLBACK(newtask_clicked_cbk), NULL);
    gtk_toolbar_insert(w_toolbar, ti_newtask, 0);

    // Refresh tasks button.
    ti_refresh_icon = gtk_image_new_from_icon_name("gtk-refresh", GTK_ICON_SIZE_SMALL_TOOLBAR);
    ti_refresh = gtk_tool_button_new(ti_refresh_icon, "Refresh");
    gtk_tool_item_set_tooltip_text(ti_refresh, "Refresh tasks from Taskwarrior database");
    g_signal_connect(ti_refresh, "clicked", G_CALLBACK(refresh_clicked_cbk), NULL);
    gtk_toolbar_insert(w_toolbar, ti_refresh, 1);

    // Separator.
    ti_sep1 = gtk_separator_tool_item_new();
    gtk_separator_tool_item_set_draw(GTK_SEPARATOR_TOOL_ITEM(ti_sep1), 1);
    gtk_toolbar_insert(w_toolbar, ti_sep1, 2);

    // Save task button.
    ti_savetask_icon = gtk_image_new_from_icon_name("gtk-save", GTK_ICON_SIZE_SMALL_TOOLBAR);
    ti_savetask = gtk_tool_button_new(ti_savetask_icon, "Save task");
    gtk_tool_item_set_tooltip_text(ti_savetask, "Saves task to Taskwarrior database");
    g_signal_connect(ti_savetask, "clicked", G_CALLBACK(tasksave_clicked_cbk), NULL);
    gtk_toolbar_insert(w_toolbar, ti_savetask, 3);

    // Cancel task button.
    ti_canceltask_icon = gtk_image_new_from_icon_name("gtk-cancel", GTK_ICON_SIZE_SMALL_TOOLBAR);
    ti_canceltask = gtk_tool_button_new(ti_canceltask_icon, "Cancel task");
    gtk_tool_item_set_tooltip_text(ti_canceltask, "Cancels task");
    g_signal_connect(ti_canceltask, "clicked", G_CALLBACK(taskcancel_clicked_cbk), NULL);
    gtk_toolbar_insert(w_toolbar, ti_canceltask, 4);

    // Mark task as done button.
    ti_taskdone_icon = gtk_image_new_from_icon_name("gtk-yes", GTK_ICON_SIZE_SMALL_TOOLBAR);
    ti_taskdone = gtk_tool_button_new(ti_taskdone_icon, "Mark task as done");
    gtk_tool_item_set_tooltip_text(ti_taskdone, "Marking task as done");
    g_signal_connect(ti_taskdone, "clicked", G_CALLBACK(taskdone_clicked_cbk), NULL);
    gtk_toolbar_insert(w_toolbar, ti_taskdone, 5);

    // Remove task button.
    ti_removetask_icon = gtk_image_new_from_icon_name("gtk-remove", GTK_ICON_SIZE_SMALL_TOOLBAR);
    ti_removetask = gtk_tool_button_new(ti_removetask_icon, "Remove task");
    gtk_tool_item_set_tooltip_text(ti_removetask, "Removes task");
    g_signal_connect(ti_removetask, "clicked", G_CALLBACK(taskcancel_clicked_cbk), NULL);
    gtk_toolbar_insert(w_toolbar, ti_removetask, 6);
}