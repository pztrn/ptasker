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
#include <sys/stat.h>

#include <json/json.h>

#include <log.h>
#include "note.h"
#include <pstr.h>
#include "tw.h"

static char *task_exec(char *opts)
{
	FILE *f;
	int ret;
	size_t s;
	char *str, *tmp, *cmd, buf[1024];

	cmd = malloc(strlen("task ") + strlen(opts) + 1);
	strcpy(cmd, "task ");
	strcat(cmd, opts);

	log_debug("execute: %s", cmd);

	f = popen(cmd, "r");

	free(cmd);

	if (!f) {
		perror("popen");
		return NULL;
	}

	str = strdup("");
	while ((s = fread(buf, 1, 1024, f))) {
		tmp = malloc(strlen(str) + s + (size_t)1);
		memcpy(tmp, str, strlen(str));
		memcpy(tmp + strlen(str), buf, s);
		tmp[strlen(str) + s] = '\0';
		free(str);
		str = tmp;
	}

	ret = pclose(f);

	if (ret == -1)
		log_err("pclose fails");

	return str;
}

static char *task_get_version()
{
	char *out;

	out = task_exec("--version");

	trim(out);

	return out;
}

static int task_check_version()
{
	char *ver;

	ver = task_get_version();

	if (!ver)
		return 0;

	log_debug("task version: %s", ver);

	if (!strcmp(ver, "2.2.0") || !strcmp(ver, "2.0.0"))
		return 1;
	else
		return 0;
}

static char *tw_exec(char *opts)
{
	char *opts2;

	if (!task_check_version()) {
		log_err("ptask is not compatible with the installed version of"
			" taskwarrior.");
		return NULL;
	}

	opts2 = malloc(strlen("rc.confirmation:no ")
		       + strlen(opts)
		       + 1);
	strcpy(opts2, "rc.confirmation:no ");
	strcat(opts2, opts);

	return task_exec(opts2);
}

static struct json_object *task_exec_json(const char *opts)
{
	struct json_object *o;
	char *str, *cmd;

	cmd = malloc(strlen("rc.json.array=on ") + strlen(opts) + 1);
	strcpy(cmd, "rc.json.array=on ");
	strcat(cmd, opts);

	str = tw_exec(cmd);

	if (str) {
		o = json_tokener_parse(str);
		free(str);
	} else {
		o = NULL;
	}

	free(cmd);

	return o;
}

struct task **tw_get_all_tasks(const char *status)
{
	int i, n;
	struct json_object *jtasks, *jtask, *json;
	struct task **tasks;
	char *opts;

	opts = malloc(strlen("export status:") + strlen(status) + 1);

	strcpy(opts, "export status:");
	strcat(opts, status);

	jtasks = task_exec_json(opts);
	free(opts);

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

		json = json_object_object_get(jtask, "priority");
		if (json)
			tasks[i]->priority
				= strdup(json_object_get_string(json));
		else
			tasks[i]->priority = strdup("");

		json = json_object_object_get(jtask, "uuid");
		tasks[i]->uuid = strdup(json_object_get_string(json));

		tasks[i]->note = note_get(tasks[i]->uuid);
	}

	tasks[n] = NULL;

	json_object_put(jtasks);

	return tasks;
}

static char *escape(const char *txt)
{
	char *result;
	char *c;

	result = malloc(2*strlen(txt)+1);
	c = result;

	while (*txt) {
		switch (*txt) {
		case '"':
		case '$':
		case '&':
		case '<':
		case '>':
			*c = '\\'; c++;
			*c = *txt;
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

void tw_modify_description(const char *uuid, const char *newdesc)
{
	char *str;
	char *opts;

	str = escape(newdesc);

	opts = malloc(1
		      + strlen(uuid)
		      + strlen(" modify :\"")
		      + strlen(str)
		      + strlen("\"")
		      + 1);
	sprintf(opts, " %s modify \"%s\"", uuid, str);

	tw_exec(opts);

	free(str);
	free(opts);
}

void tw_modify_project(const char *uuid, const char *newproject)
{
	char *str;
	char *opts;

	str = escape(newproject);

	opts = malloc(1
		      + strlen(uuid)
		      + strlen(" modify project:\"")
		      + strlen(str)
		      + strlen("\"")
		      + 1);
	sprintf(opts, " %s modify project:\"%s\"", uuid, str);

	tw_exec(opts);

	free(str);
	free(opts);
}

void tw_modify_priority(const char *uuid, const char *priority)
{
	char *str;
	char *opts;

	str = escape(priority);

	opts = malloc(1
		      + strlen(uuid)
		      + strlen(" modify priority:\"")
		      + strlen(str)
		      + strlen("\"")
		      + 1);
	sprintf(opts, " %s modify priority:\"%s\"", uuid, str);

	tw_exec(opts);

	free(str);
	free(opts);
}

void tw_add(const char *newdesc, const char *prj, const char *prio)
{
	char *opts, *eprj;

	eprj = escape(prj);

	opts = malloc(strlen(" add")
		      + strlen(" priority:")
		      + 1
		      + strlen(" project:\\\"")
		      + strlen(eprj)
		      + strlen("\\\"")
		      + strlen(" \"")
		      + strlen(newdesc)
		      + strlen("\"")
		      + 1);

	strcpy(opts, " add");

	if (prio && strlen(prio) == 1) {
		strcat(opts, " priority:");
		strcat(opts, prio);
	}

	if (eprj && strlen(prj)) {
		strcat(opts, " project:\\\"");
		strcat(opts, eprj);
		strcat(opts, "\\\"");
	}

	strcat(opts, " \"");
	strcat(opts, newdesc);
	strcat(opts, " \"");

	tw_exec(opts);

	free(opts);
	free(eprj);
}

void tw_done(const char *uuid)
{
	char *opts;

	opts = malloc(1
		      + strlen(uuid)
		      + strlen(" done")
		      + 1);
	sprintf(opts, " %s done", uuid);

	tw_exec(opts);

	free(opts);
}

static void task_free(struct task *task)
{
	if (!task)
		return ;

	free(task->description);
	free(task->status);
	free(task->uuid);
	free(task->note);
	free(task->project);
	free(task->priority);

	free(task);
}

void tw_task_list_free(struct task **tasks)
{
	struct task **cur;

	if (!tasks)
		return ;

	for (cur = tasks; *cur; cur++)
		task_free(*cur);

	free(tasks);
}
