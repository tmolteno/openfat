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

/* FAT Filesystem implementation, public interface
 */

#ifndef __OPENFAT_H
#define __OPENFAT_H

#include <stdint.h>

#include <sys/stat.h>

#include "blockdev.h"

/* Forward declarations of private structures. */
struct fat_vol_handle;
struct fat_file_handle;

int fat_vol_init(const struct block_device *dev, struct fat_vol_handle *h);

int fat_path_open(const struct fat_vol_handle *fat, const char *path,
		struct fat_file_handle *h);
void fat_file_seek(struct fat_file_handle *h, uint32_t offset);
int fat_file_read(struct fat_file_handle *h, void *buf, int size);

int fat_file_stat(struct fat_file_handle *h, struct stat *st);

struct dirent {
	char d_name[256];
};

int fat_dir_read(struct fat_file_handle *dir, struct dirent *ent);
int fat_dir_open_file(const struct fat_file_handle *dir, const char *name,
		struct fat_file_handle *file);

/* Everything below is private.  Applications should not direcly access
 * anything here.
 */
struct fat_vol_handle {
	const struct block_device *dev;
	/* FAT type: 12, 16 or 32 */
	int type;
	/* Useful fields from BPB */
	uint16_t bytes_per_sector;
	uint8_t sectors_per_cluster;
	uint16_t reserved_sector_count;
	/* Fields calcuated from BPB */
	uint32_t first_data_sector;
	uint32_t cluster_count;
	union {
		struct {
			uint32_t root_cluster;
		} fat32;
		struct {
			uint16_t root_sector_count;
			uint16_t root_first_sector;
		} fat12_16;
	};
};

struct fat_file_handle {
	const struct fat_vol_handle *fat;
	/* Fields from dir entry */
	uint32_t size;
	uint32_t first_cluster;
	/* Internal state information */
	uint32_t position;
	uint32_t cur_cluster;	/* This is used for sector on FAT12/16 root */
	uint8_t root_flag;	/* Flag to mark root directory on FAT12/16 */
	/* Reference to dirent */
	uint32_t dirent_sector;
	uint16_t dirent_offset;
};

#endif
