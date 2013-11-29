/*
 * Copyright (C) 2010-2013 jeanfi@gmail.com
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

#include <stdlib.h>
#include <string.h>

#include "list.h"

int list_length(void **list)
{
	int n;

	if (!list)
		return 0;

	n = 0;
	while (*list) {
		n++;
		list++;
	}

	return n;
}

void **list_add(void **list, void *item)
{
	int n;
	void **result;

	n = list_length(list);

	result = malloc((n + 1 + 1) * sizeof(void *));

	if (list)
		memcpy(result, list, n * sizeof(void *));

	result[n] = item;
	result[n + 1] = NULL;

	return result;
}

void list_free(void **list)
{
	free(list);
}
