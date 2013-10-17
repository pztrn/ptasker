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
#include <sys/types.h>

#include <log.h>
#include <note.h>

static char *get_path(const char *uuid)
{
	char *home, *dir, *path;

	home = getenv("HOME");

	if (!home) {
		log_err("HOME environment variable not defined");
		return NULL;
	}

	dir = malloc(strlen(home) + 1 + strlen(".task") + 1);
	sprintf(dir, "%s/%s", home, ".task");
	mkdir(dir, 0777);

	path = malloc(strlen(dir) + 1 + strlen(uuid) + strlen(".note") + 1);
	sprintf(path, "%s/%s.note", dir, uuid);

	free(dir);

	return path;
}

void note_put(const char *uuid, const char *note)
{
	char *path;
	FILE *f;

	path = get_path(uuid);

	if (!path)
		return ;

	log_debug("note_put %s %s %s", path, uuid, note);

	f = fopen(path, "w");

	if (f) {
		fwrite(note, 1, strlen(note), f);
		fclose(f);
	} else {
		log_err("Failed to open %s", path);
	}

	free(path);
}

char *note_get(const char *uuid)
{
	char *str, *tmp, *path;
	FILE *f;
	char buf[1024];
	size_t s;

	path = get_path(uuid);

	if (!path)
		return NULL;

	str = strdup("");

	f = fopen(path, "r");

	if (f) {
		while ((s = fread(buf, 1, 1024, f))) {
			tmp = malloc(strlen(str) + s + (size_t)1);
			memcpy(tmp, str, strlen(str));
			memcpy(tmp + strlen(str), buf, s);
			tmp[strlen(str) + s] = '\0';
			free(str);
			str = tmp;
		}
		fclose(f);
	} else {
		log_err("Failed to open %s", path);
	}

	return str;
}
