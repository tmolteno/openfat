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

/* FAT Filesystem core implementation, private interface
 */
#ifndef __FAT_CORE_H
#define __FAT_CORE_H

#include "bpb.h"
#include "direntry.h"

extern uint8_t _fat_sector_buf[];

static inline uint32_t
fat_eoc(const struct fat_vol_handle *fat) 
{
	switch (fat->type) {
	case FAT_TYPE_FAT12:
		return 0x0FF8;
	case FAT_TYPE_FAT16:
		return 0xFFF8;
	case FAT_TYPE_FAT32:
		return 0x0FFFFFF8;
	}
	return -1;
}

static inline uint32_t 
fat_first_sector_of_cluster(const struct fat_vol_handle *fat, uint32_t n)
{
	return ((n - 2) * fat->sectors_per_cluster) + fat->first_data_sector;
}

uint32_t 
_fat_get_next_cluster(const struct fat_vol_handle *h, uint32_t cluster);

void _fat_file_root(const struct fat_vol_handle *fat, struct fat_file_handle *h);
void _fat_file_init(const struct fat_vol_handle *fat, const struct fat_sdirent *, 
		struct fat_file_handle *h);

int _fat_dir_create_file(struct fat_vol_handle *vol, const char *name,
		uint8_t attr, struct fat_file_handle *file);

#define FAT_GET_SECTOR(fat, sector)	do {\
	if(block_read_sectors((fat)->dev, (sector), 1, _fat_sector_buf) != 1) \
		return -EIO; \
} while(0)

#define FAT_PUT_SECTOR(fat, sector)	do {\
	if(block_write_sectors((fat)->dev, (sector), 1, _fat_sector_buf) != 1) \
		return -EIO; \
} while(0)

#endif

