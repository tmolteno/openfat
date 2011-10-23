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
#include <fcntl.h>

#include <assert.h>

#include <sys/stat.h>

#include "openfat.h"

/* Prototypes for blockdev_file.c functions */
extern struct block_device * 
block_device_file_new(const char *filename, const char *mode);
extern void block_device_file_destroy(struct block_device *bldev);

void print_tree(struct fat_vol_handle *vol, struct fat_file_handle *dir, 
		const char *path)
{
	struct dirent ent;
	char tmppath[1024];
	struct fat_file_handle subdir;

	while(!fat_readdir(dir, &ent)) {
		if((strcmp(ent.d_name, ".") == 0) || 
		   (strcmp(ent.d_name, "..") == 0))
			continue;
		sprintf(tmppath, "%s/%s", path, ent.d_name);
		puts(tmppath);
		
		if(ent.fat_attr == FAT_ATTR_DIRECTORY) {
			fat_chdir(vol, ent.d_name);
			assert(fat_open(vol, ".", 0, &subdir) == 0);
			print_tree(vol, &subdir, tmppath);
			fat_chdir(vol, "..");
		}
	}

}

int main(int argc, char *argv[])
{
	struct block_device *bldev;
	FatVol vol;
	FatFile file;
	char *rootpath = argc > 2 ? argv[2] : "/";

	bldev = block_device_file_new(argc > 1 ? argv[1] : "fat32.img", "r+");
	assert(bldev != NULL);

	assert(fat_vol_init(bldev, &vol) == 0);
	fprintf(stderr, "Fat type is FAT%d\n", vol.type);

	fat_mkdir(&vol, "Directory1");
	fat_mkdir(&vol, "Directory2");
	fat_mkdir(&vol, "Directory3");
	assert(fat_chdir(&vol, "Directory1") == 0);
	fat_mkdir(&vol, "Directory1");
	fat_mkdir(&vol, "Directory2");
	fat_mkdir(&vol, "Directory3");
	if(fat_create(&vol, "Message file with a long name.txt", O_WRONLY, &file) == 0) {
		for(int i = 0; i < 100; i++) {
			char message[80];
			sprintf(message, "Here is a message %d\n", i);
			assert(fat_write(&file, message, strlen(message)) == (int)strlen(message));
		}
	}
	assert(fat_chdir(&vol, "..") == 0);
	assert(fat_open(&vol, ".", O_RDONLY, &file) == 0);
	print_tree(&vol, &file, rootpath[0] == '/' ? rootpath + 1 : rootpath);

	block_device_file_destroy(bldev);
}

