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

/* FAT Filesystem core implementation, public interface
 */
#ifndef __FAT_H
#define __FAT_H

#include "direntry.h"

struct fat_file_handle {
	struct fat_handle *fat;
	/* Fields from dir entry */
	uint32_t size;
	uint32_t first_cluster;
	/* Internal state information */
	uint32_t position;
	uint32_t cur_cluster;
};

struct fat_handle {
	struct block_device *dev;
	/* FAT type: 12, 16 or 32 */
	int type;
	/* Useful fields from BPB */
	uint16_t bytes_per_sector;
	uint8_t sectors_per_cluster;
	uint16_t reserved_sector_count;
	/* Fields calcuated from BPB */
	uint32_t first_data_sector;
	union {
		struct {
			uint32_t root_cluster;
		} fat32;
	};
	/* Interal state information */
	struct fat_file_handle current_dir;
};

int fat_init(struct block_device *dev, struct fat_handle *h);
int fat_file_init(struct fat_handle *fat, struct fat_dirent *, 
		struct fat_file_handle *h);
void fat_file_rewind(struct fat_file_handle *h);
int fat_file_read(struct fat_file_handle *h, void *buf, int size);

#endif

