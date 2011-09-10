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

/* Example to list directory structure.
 */

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include <assert.h>

#include "fat.h"
#include "direntry.h"


/* Prototypes for blockdev_file.c functions */
extern struct block_device * 
block_device_file_new(const char *filename, const char *mode);
extern void block_device_file_destroy(struct block_device *bldev);

void print_tree(struct fat_file_handle *dir, int nest)
{
	struct dirent ent;

	while(fat_dir_read(dir, &ent)) {
		for(int i = 0; i < nest; i++) printf("\t");
		printf("%02x - %s\n", ent.fatent.attr, ent.d_name);
		
		if((ent.fatent.attr == FAT_ATTR_DIRECTORY) && (ent.d_name[0] != '.')) {
			struct fat_file_handle subdir;
			fat_file_init(dir->fat, &ent.fatent, &subdir);
			print_tree(&subdir, nest + 1);
		}
	}

}

int main(int argc, char *argv[])
{
	struct block_device *bldev;
	struct fat_vol_handle fat;
	struct fat_file_handle root;

	bldev = block_device_file_new(argc > 1 ? argv[1] : "fat32.img", "r");
	assert(bldev != NULL);

	fat_vol_init(bldev, &fat);
	fat_file_root(&fat, &root);

	printf("Fat type is FAT%d\n", fat.type);

	print_tree(&root, 0);

	block_device_file_destroy(bldev);
}

