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

/* FAT Filesystem core implementation
 */

#include <stdint.h>
#include <string.h>

#include "blockdev.h"
#include "leaccess.h"
#include "bpb.h"
#include "fat.h"

/* Build time configuration */
#define MAX_SECTOR_SIZE 512

static uint8_t sector_buf[MAX_SECTOR_SIZE];

int fat_vol_init(const struct block_device *dev, struct fat_vol_handle *h)
{
	struct bpb_common *bpb = (void *)&sector_buf;

	memset(h, 0, sizeof(*h));
	h->dev = dev;
	
	block_read_sectors(dev, 0, 1, sector_buf);
	h->type = fat_type(bpb);
	h->bytes_per_sector = __get_le16(&bpb->bytes_per_sector);
	h->sectors_per_cluster = bpb->sectors_per_cluster;
	h->first_data_sector = _bpb_first_data_sector(bpb);
	h->reserved_sector_count = __get_le16(&bpb->reserved_sector_count);
	if(h->type == FAT_TYPE_FAT32) {
		struct bpb_fat32 *bpb32 = (void *)&sector_buf;
		h->fat32.root_cluster = __get_le32(&bpb32->root_cluster);
	} else {
		h->fat12_16.root_sector_count = _bpb_root_dir_sectors(bpb);
		h->fat12_16.root_first_sector = _bpb_first_data_sector(bpb) - 
					h->fat12_16.root_sector_count;
	}

	return 0;
}

static uint32_t 
fat_get_next_cluster(const struct fat_vol_handle *h, uint32_t cluster)
{
	uint32_t offset;
	uint32_t sector;

	if(h->type == FAT_TYPE_FAT12)
		offset = cluster + (cluster / 2);
	else if(h->type == FAT_TYPE_FAT16)
		offset = cluster * 2;
	else if(h->type == FAT_TYPE_FAT32)
		offset = cluster * 4;

	sector = h->reserved_sector_count + (offset / h->bytes_per_sector);
	offset %= h->bytes_per_sector;

	block_read_sectors(h->dev, sector, 1, sector_buf); 

	if(h->type == FAT_TYPE_FAT12) {
		uint32_t next;
		if(offset == (uint32_t)h->bytes_per_sector - 1) {
			/* Fat entry is over sector boundary */
			next = sector_buf[offset];
			block_read_sectors(h->dev, sector + 1, 1, sector_buf); 
			next += sector_buf[0] << 8;
		} else {
			next = __get_le16((uint16_t*)(sector_buf + offset));
		}
		if(cluster & 1) 
			return next >> 4;
		else
			return next & 0xFFF;
	} else if(h->type == FAT_TYPE_FAT16) {
		return __get_le16((uint16_t*)(sector_buf + offset));
	} else if(h->type == FAT_TYPE_FAT32) {
		return __get_le32((uint32_t*)(sector_buf + offset)) & 0x0FFFFFFF;
	}
	/* We shouldn't get here... */
	return 0;
}

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

int fat_file_init(const struct fat_vol_handle *fat, 
		const struct fat_dirent *dirent, 
		struct fat_file_handle *h)
{
	memset(h, 0, sizeof(*h));
	h->fat = fat;

	if(dirent == NULL) { /* special case to open the root directory */
		if(fat->type == FAT_TYPE_FAT32) {
			h->first_cluster = fat->fat32.root_cluster;
		} else {
			/* FAT12/FAT16 root directory */
			h->root_flag = 1;
			h->first_cluster = fat->fat12_16.root_first_sector;
			h->size = h->fat->fat12_16.root_sector_count * h->fat->bytes_per_sector;
		}
	} else {
		h->first_cluster = ((uint32_t)__get_le16(&dirent->cluster_hi) << 16) | 
				__get_le16(&dirent->cluster_lo);
		h->size = __get_le32(&dirent->size);
	}

	h->cur_cluster = h->first_cluster;

	return 0;
}

void fat_file_seek(struct fat_file_handle *h, uint32_t offset)
{
	h->cur_cluster = h->first_cluster;

	if(h->size && (offset > h->size))
		offset = h->size;

	h->position = offset;

	if(h->root_flag) { /* FAT12/16 root dir isn't a cluster chain */
		return;
	}

	/* Interate over cluster chain to find cluster */
	while(offset >= (h->fat->sectors_per_cluster * h->fat->bytes_per_sector)) {
		h->cur_cluster = fat_get_next_cluster(h->fat, h->cur_cluster);
		offset -= h->fat->sectors_per_cluster * h->fat->bytes_per_sector;
	}

}

#define MIN(x, y) (((x) < (y))?(x):(y))
int fat_file_read(struct fat_file_handle *h, void *buf, int size)
{
	int i;
	uint32_t sector;
	uint16_t offset;

	if(h->cur_cluster == fat_eoc(h->fat)) 
		return 0;

	if(h->root_flag) {
		sector = h->cur_cluster + 
			(h->position / h->fat->bytes_per_sector);
	} else {
		sector = fat_first_sector_of_cluster(h->fat, h->cur_cluster);
		sector += (h->position / h->fat->bytes_per_sector) % h->fat->sectors_per_cluster;
	}
	offset = h->position % h->fat->bytes_per_sector;
	if(h->size && ((h->position + size) > h->size))
		size = h->size - h->position;

	for(i = 0; i < size; ) {
		uint16_t chunk = MIN(h->fat->bytes_per_sector - offset, size - i);
		block_read_sectors(h->fat->dev, sector, 1, sector_buf); 
		memcpy(buf + i, sector_buf + offset, chunk);
		h->position += chunk;
		i += chunk;
		if((h->position % h->fat->bytes_per_sector) != 0) 
			/* we didn't read until the end of the sector... */
			break;
		offset = 0;
		sector++;
		if(h->root_flag) /* FAT12/16 isn't a cluster chain */
			continue;
		if((sector % h->fat->sectors_per_cluster) == 0) {
			/* Go to next cluster... */
			h->cur_cluster = fat_get_next_cluster(h->fat, 
						h->cur_cluster);
			if(h->cur_cluster == fat_eoc(h->fat)) 
				return i;
			sector = fat_first_sector_of_cluster(h->fat, 
						h->cur_cluster);
		}
	}

	return i;
}

