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
#include "ui_newtask_dialog.h"
#include "ui_taskpanel.h"
#include "ui_tasktree.h"

static GtkWidget *ti_status_cb;

static GtkWidget *ti_search_entry;

// Callback for new task button click.
int newtask_clicked_cbk(GtkButton *btn, gpointer data)
{
    ui_newtask_dialog();

    return FALSE;
}

int refresh_clicked_cbk(GtkButton *btn, gpointer data)
{
    log_fct_enter();
    refresh();
    log_fct_exit();
    return FALSE;
}

static int status_changed_cbk(GtkComboBoxText *w, gpointer data)
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

    if (current_task)
    {
        log_fct("uuid=%d", current_task->uuid);
        tw_task_remove(current_task->uuid);
        refresh();
    }

    return FALSE;
}

static int tasksave_clicked_cbk(GtkButton *btn, gpointer data)
{
    taskpanel_save();

    refresh();

    return FALSE;
}

const char *ui_toolbar_get_status_filter()
{
    const char *status;

    log_fct_enter();

    status = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(ti_status_cb));
    log_info("status: %s", status);

    log_fct_exit();

    return status;
}

void ui_toolbar_search_field_changed(GtkEntry *entry, gchar *preedit, gpointer data)
{
    const char *search_for;
    search_for = gtk_entry_get_text(GTK_ENTRY(ti_search_entry));
    log_info("Starting searching for '%s'...", search_for);
    ui_tasktree_search_for(search_for);
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
    // Mark task as done.
    GtkToolItem *ti_taskdone;
    GtkWidget *ti_taskdone_icon;
    // Remove task button.
    GtkToolItem *ti_removetask;
    GtkWidget *ti_removetask_icon;
    // One more separator.
    GtkToolItem *ti_sep2;
    // Status filtering.
    GtkToolItem *ti_status_label_item;
    GtkWidget *ti_status_label;
    GtkToolItem *ti_status_cb_item;
    // Expanding separator.
    GtkToolItem *ti_sep_exp;
    // Search bar.
    GtkToolItem *ti_search;

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

    // Mark task as done button.
    ti_taskdone_icon = gtk_image_new_from_icon_name("gtk-yes", GTK_ICON_SIZE_SMALL_TOOLBAR);
    ti_taskdone = gtk_tool_button_new(ti_taskdone_icon, "Mark task as done");
    gtk_tool_item_set_tooltip_text(ti_taskdone, "Marking task as done");
    g_signal_connect(ti_taskdone, "clicked", G_CALLBACK(taskdone_clicked_cbk), NULL);
    gtk_toolbar_insert(w_toolbar, ti_taskdone, 4);

    // Remove task button.
    ti_removetask_icon = gtk_image_new_from_icon_name("gtk-remove", GTK_ICON_SIZE_SMALL_TOOLBAR);
    ti_removetask = gtk_tool_button_new(ti_removetask_icon, "Remove task");
    gtk_tool_item_set_tooltip_text(ti_removetask, "Removes task");
    g_signal_connect(ti_removetask, "clicked", G_CALLBACK(taskremove_clicked_cbk), NULL);
    gtk_toolbar_insert(w_toolbar, ti_removetask, 5);

    // One more separator :)
    ti_sep2 = gtk_separator_tool_item_new();
    gtk_separator_tool_item_set_draw(GTK_SEPARATOR_TOOL_ITEM(ti_sep2), 1);
    gtk_toolbar_insert(w_toolbar, ti_sep2, 6);

    // Status filtering.
    ti_status_label_item = gtk_tool_item_new();
    ti_status_label = gtk_label_new("Status:");
    gtk_container_add(GTK_CONTAINER(ti_status_label_item), GTK_WIDGET(ti_status_label));
    gtk_toolbar_insert(w_toolbar, ti_status_label_item, 7);

    ti_status_cb_item = gtk_tool_item_new();
    ti_status_cb = gtk_combo_box_text_new();
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(ti_status_cb), NULL, "pending");
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(ti_status_cb), NULL, "completed");
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(ti_status_cb), NULL, "deleted");
    gtk_combo_box_set_active(GTK_COMBO_BOX(ti_status_cb), 1);
    gtk_container_add(GTK_CONTAINER(ti_status_cb_item), GTK_WIDGET(ti_status_cb));
    gtk_toolbar_insert(w_toolbar, ti_status_cb_item, 8);
    g_signal_connect(ti_status_cb,
                     "changed", (GCallback)status_changed_cbk,
                     NULL);
    gtk_combo_box_set_active(GTK_COMBO_BOX(ti_status_cb), 0);

    // Expanding separator.
    ti_sep_exp = gtk_separator_tool_item_new();
    gtk_separator_tool_item_set_draw(GTK_SEPARATOR_TOOL_ITEM(ti_sep2), 0);
    gtk_tool_item_set_expand(GTK_TOOL_ITEM(ti_sep_exp), 1);
    gtk_toolbar_insert(w_toolbar, ti_sep_exp, 9);

    // Search field.
    ti_search = gtk_tool_item_new();
    ti_search_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(ti_search_entry), "Search for tasks...");
    gtk_entry_set_icon_from_icon_name(GTK_ENTRY(ti_search_entry), GTK_ENTRY_ICON_PRIMARY, "gtk-find");
    gtk_entry_set_width_chars(GTK_ENTRY(ti_search_entry), 25);
    gtk_container_add(GTK_CONTAINER(ti_search), GTK_WIDGET(ti_search_entry));
    g_signal_connect(ti_search_entry,
                     "changed", (GCallback)ui_toolbar_search_field_changed, NULL);
    gtk_toolbar_insert(w_toolbar, ti_search, 10);
}