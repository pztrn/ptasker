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


char *task_exec(char *opts)
{
	FILE *f;
	int ret;
	size_t s;
	char *str, *tmp, *cmd, buf[1024];

	cmd = malloc(strlen("task rc.json.array=on ") + strlen(opts) + 1);
	strcpy(cmd, "task rc.json.array=on ");
	strcat(cmd, opts);

	printf("execute: %s\n", cmd);

	f = popen(cmd, "r");

	if (!f) {
		perror("popen");
		str = NULL;
		goto exit_free;
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

	if (ret == -1) {
		printf("pclose fails\n");
		perror("pclose");
	}

 exit_free:
	free(cmd);

	return str;
}

#include <json/json.h>

#include "tw.h"

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

struct task **tw_get_all_tasks(const char *status)
{
	int i, n;
	struct json_object *jtasks, *jtask, *json;
	struct task **tasks;
	char *opts;

	opts = malloc(strlen("export status:") + strlen(status) + 1);
	sprintf(opts, "export status:%s", status);

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

		tasks[i]->note = NULL;
	}

	tasks[n] = NULL;

	json_object_put(jtasks);

	return tasks;
}

char *escape(const char *txt)
{
	char *result;
	char *c;

	result = malloc(2*strlen(txt)+1);
	c = result;

	while (*txt) {
		switch (*txt) {
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

	task_exec(opts);

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

	task_exec(opts);

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

	task_exec(opts);

	free(str);
	free(opts);
}

void tw_add(const char *newdesc)
{
	char *str;
	char *opts;

	str = escape(newdesc);

	opts = malloc(1
		      + strlen(" add \"")
		      + strlen(str)
		      + strlen("\"")
		      + 1);
	sprintf(opts, " add \"%s\"", str);

	task_exec(opts);

	free(str);
	free(opts);
}

void tw_done(const char *uuid)
{
	char *opts;

	opts = malloc(1
		      + strlen(uuid)
		      + strlen(" done")
		      + 1);
	sprintf(opts, " %s done", uuid);

	task_exec(opts);

	free(opts);
}
