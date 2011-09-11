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

#include <string.h>

#include "fat.h"
#include "direntry.h"

int fat_path_open(const struct fat_vol_handle *fat, const char *path,
		struct fat_file_handle *h)
{
	if(!path || (path[0] == 0)) 
		return 0;

	if(path[0] == '/') {
		fat_file_root(fat, h);
		path++;
	} else {
		/* TODO: Handle relative path */
		fat_file_root(fat, h);
	}

	while(path && *path) {
		if(!fat_dir_open_file(h, path, h)) 
			return 0;
		path = strchr(path, '/');
		if(path) path++;
	};

	return 1;
}

