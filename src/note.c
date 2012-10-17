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
#include <sys/stat.h>
#include <sys/types.h>

#include <note.h>

void note_put(const char *uuid, const char *note)
{
	char *home, *dir, *path;
	FILE *f;

	home = getenv("HOME");

	if (!home)
		return ;

	dir = malloc(strlen(home) + 1 + strlen(".task") + 1);
	sprintf(dir, "%s/%s", home, ".task");
	mkdir(dir, 0777);

	path = malloc(strlen(dir) + 1 + strlen(uuid) + strlen(".note") + 1);
	sprintf(path, "%s/%s.note", dir, uuid);

	printf("note_put %s %s %s\n", path, uuid, note);

	f = fopen(path, "w");

	if (f) {
		fwrite(note, 1, strlen(note), f);
		fclose(f);
	} else {
		fprintf(stderr, "Failed to open %s\n", path);
	}

	free(dir);
	free(path);
}
