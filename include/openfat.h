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

#include "openfat/blockdev.h"

/* Forward declarations of private structures. */
struct fat_vol_handle;
struct fat_file_handle;

/* Mount a FAT volume. */
int __attribute__((warn_unused_result))
fat_vol_init(const struct block_device *dev, struct fat_vol_handle *h);

/* Change current working directory */
int fat_chdir(struct fat_vol_handle *vol, const char *name);
/* Create a new directory */
int fat_mkdir(struct fat_vol_handle *vol, const char *name);
/* Remove an empty directory */
int fat_rmdir(struct fat_vol_handle *vol, const char *name); /* TODO */

/* Open a file */
int __attribute__((warn_unused_result))
fat_open(struct fat_vol_handle *vol, const char *name, int flags,
		  struct fat_file_handle *file);
/* Read from a file */
int __attribute__((warn_unused_result))
fat_read(struct fat_file_handle *h, void *buf, int size);
/* Write to a file */
int __attribute__((warn_unused_result))
fat_write(struct fat_file_handle *h, const void *buf, int size);
/* Seek in a file */
off_t fat_lseek(struct fat_file_handle *h, off_t offset, int whence);
/* Unlink/delete a file */
int fat_unlink(struct fat_vol_handle *vol, const char *name);

#define FAT_ATTR_READ_ONLY	0x01
#define FAT_ATTR_HIDDEN		0x02
#define FAT_ATTR_SYSTEM		0x04
#define FAT_ATTR_VOLUME_ID	0x08
#define FAT_ATTR_DIRECTORY	0x10
#define FAT_ATTR_ARCHIVE	0x20
#define FAT_ATTR_LONG_NAME	0x0F

struct dirent {
	char d_name[256];
	/* Non-standard */
	uint8_t fat_attr;	/* FAT file attributes */
};

int fat_readdir(struct fat_file_handle *dir, struct dirent *ent);

/* Unix like API */
#include <stdlib.h>

struct fat_vol_handle * ufat_mount(struct block_device *dev);
static inline void ufat_umount(struct fat_vol_handle *vol) { free(vol); }

struct fat_file_handle *
ufat_open(struct fat_vol_handle *vol, const char *path, int flags);
static inline void ufat_close(struct fat_file_handle *file) { free(file); }

int ufat_stat(struct fat_file_handle *file, struct stat *stat);

#define ufat_read fat_read
#define ufat_write fat_write

#define ufat_chdir fat_chdir
#define ufat_mkdir fat_mkdir

/* Everything below is private.  Applications should not direcly access
 * anything here.
 */
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

struct fat_vol_handle {
	const struct block_device *dev;
	/* FAT type: 12, 16 or 32 */
	int type;
	/* Useful fields from BPB */
	uint16_t bytes_per_sector;
	uint8_t sectors_per_cluster;
	uint16_t reserved_sector_count;
	uint8_t num_fats;
	/* Fields calcuated from BPB */
	uint32_t first_data_sector;
	uint32_t cluster_count;
	uint32_t fat_size;
	union {
		struct {
			uint32_t root_cluster;
		} fat32;
		struct {
			uint16_t root_sector_count;
			uint16_t root_first_sector;
		} fat12_16;
	};
	/* Internal state */
	struct fat_file_handle cwd;
};

#endif
