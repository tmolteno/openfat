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

#ifndef __FAT_H
#define __FAT_H

#include <stdint.h>

/* Boot Sector / BIOS Parameter Block definitions */

/* Boot sector fields common to FAT12/FAT16/FAT32 */
struct bpb_common {
	uint8_t boot_jmp[3];
	char oem_name[8];
	uint16_t bytes_per_sector;
	uint8_t sectors_per_cluster;
	uint16_t reserved_sector_count;
	uint8_t num_fats;
	uint16_t root_entry_count;
	uint16_t total_sectors_16;
	uint8_t media;
	uint16_t fat_size_16;
	uint16_t sectors_per_track;
	uint16_t num_heads;
	uint32_t hidden_sectors;
	uint32_t total_sectors_32;
} __attribute__((packed));

/* Boot sector fields only in FAT12/FAT16 */
struct bpb_fat12_16 {
	struct bpb_common common;
	uint8_t drive_num;
	uint8_t Reserved1;
	uint8_t boot_sig;
	uint32_t volume_id;
	char volume_label[11];
	char fs_type[8];
} __attribute__((packed));

/* Boot sector fields only in FAT32 */
struct bpb_fat32 {
	struct bpb_common common;
	uint32_t fat_size_32;
	uint16_t ext_flags;
	uint16_t fs_version;
	uint32_t root_cluster;
	uint16_t fs_info;
	uint16_t bk_boot_sec;
	uint8_t Reserved[12];
	uint8_t drive_num;
	uint8_t Reserved1;
	uint8_t boot_sig;
	uint32_t volume_id;
	char volume_label[11];
	char fs_type[8];
} __attribute__((packed));


static inline uint32_t 
fat_root_dir_sectors(struct bpb_common *bpb)
{
	return ((bpb->root_entry_count * 32) + (bpb->bytes_per_sector - 1)) /
		bpb->bytes_per_sector;
}

static inline uint32_t
fat_size(struct bpb_common *bpb)
{
	return bpb->fat_size_16 ? bpb->fat_size_16 : 
		((struct bpb_fat32 *)bpb)->fat_size_32;
}

static inline uint32_t 
fat_first_data_sector(struct bpb_common *bpb)
{
	return bpb->reserved_sector_count + (bpb->num_fats + fat_size(bpb)) 
		+ fat_root_dir_sectors(bpb);
}

static inline uint32_t
fat_first_sector_of_cluster(struct bpb_common *bpb, uint32_t n)
{
	return ((n * 2) * bpb->sectors_per_cluster) + 
		fat_first_data_sector(bpb);
}

enum fat_type {
	FAT12,
	FAT16,
	FAT32,
};

/* FAT type is determined by count of clusters */
static inline enum fat_type
fat_type(struct bpb_common *bpb)
{
	uint32_t tot_sec = bpb->total_sectors_16 ?  bpb->total_sectors_16 :
			bpb->total_sectors_32;
	uint32_t data_sec = tot_sec - (bpb->reserved_sector_count + (bpb->num_fats * fat_size(bpb)) + fat_root_dir_sectors(bpb));

	uint32_t cluster_count = data_sec / bpb->sectors_per_cluster;

	if(cluster_count < 4085) {
		return FAT12;
	} else if(cluster_count < 65525) {
		return FAT16;
	} 
	return FAT32;
}

#endif

