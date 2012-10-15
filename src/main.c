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

static char *task_exec(char *opts)
{
	FILE *f;
	int ret, s;
	char *str, *tmp, *cmd, buf[1024];

	str = NULL;

	cmd = malloc(strlen("task rc.json.array=on ") + strlen(opts) + 1);
	strcpy(cmd, "task rc.json.array=on ");
	strcat(cmd, opts);

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

int main(int argc, char **argv)
{
	struct json_object *o;

	o = task_exec_json("export");

	printf("%s\n", json_object_to_json_string(o));

	exit(EXIT_SUCCESS);
}
