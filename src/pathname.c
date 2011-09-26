/*
 * This file is part of the openfat project.
 *
 * Copyright (C) 2011  Department of Physics, University of Otago
 * Written by Gareth McMullin <gareth@blacksphere.co.nz>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/* Path name support
 */
#include "openfat.h"

#include <string.h>

#include "fat_core.h"
#include "direntry.h"

int fat_path_open(struct fat_vol_handle *fat, const char *path,
		struct fat_file_handle *h)
{
	if(!path || (path[0] == 0)) 
		return -1;

	if(path[0] == '/') {
		_fat_file_root(fat, h);
		path++;
	} else {
		/* TODO: Handle relative path */
		_fat_file_root(fat, h);
	}

	while(path && *path) {
		memcpy(&fat->cwd, h, sizeof(*h));
		if(fat_open(fat, path, 0, h)) 
			return -1;
		path = strchr(path, '/');
		if(path) path++;
	};

	return 0;
}

