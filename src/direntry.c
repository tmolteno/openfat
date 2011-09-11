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

/* FAT Directory .
 */

#include <string.h>
#include <ctype.h>

#include "leaccess.h"
#include "blockdev.h"
#include "fat.h"
#include "direntry.h"

#define LONG_NAME_SUPPORT

#ifdef LONG_NAME_SUPPORT
uint8_t lfn_chksum(uint8_t *dosname)
{
	uint8_t sum = 0;
	int i;

	for (i = 0; i < 11; i++) 
		sum = ((sum & 1) ? 0x80 : 0) + (sum >> 1) + *dosname++;

	return sum;
}

/* Used to convert W95 UTF-16 filenames to ascii.
 * 0 means terminating null reached.
 * 1 means converted to end on input.
 * 2 means conversion error.
 */
static int ascii_from_utf16(char *ascii, const uint16_t *utf16, int count)
{
	uint16_t tmp;
	while(count--) {
		tmp = __get_le16(utf16++);
		if(tmp > 127) 
			return 2;
		*ascii++ = tmp;
		if(tmp == 0)
			return 0;
	}
	return 1;
}

/* Check ASCII and UTF-16 strings for equality.
 * 0 means terminating null reached.
 * 1 means matched to end on input.
 * 2 means mitmatch or conversion error.
 */
static int 
ascii_cmp_utf16(const char *ascii, const uint16_t *utf16, int count)
{
        while(count--) {
                uint16_t u = __get_le16(utf16++);

                /* Done on terminating null */
                if(((*ascii == 0) || (*ascii == '/')) && (u == 0))
                        return 0;

                /* Check for mismatch or non-ascii */
                if((u > 127) || (toupper(*ascii) != toupper(u)))
                        return 2;

		ascii++;
        }
        return 1;
}
#endif

int fat_dir_read(struct fat_file_handle *h, struct dirent *ent)
{
#ifdef LONG_NAME_SUPPORT
	uint16_t csum = -1;
#endif
	uint32_t sector;
	uint16_t offset;
	int i, j;

	if(h->cur_cluster == fat_eoc(h->fat)) 
		return 0;

	if(h->root_flag) {
		/* FAT12/FAT16 root directory */
		sector = h->cur_cluster + 
			(h->position / h->fat->bytes_per_sector);
	} else {
		sector = fat_first_sector_of_cluster(h->fat, h->cur_cluster);
		sector += (h->position / h->fat->bytes_per_sector) % h->fat->sectors_per_cluster;
	}
	offset = h->position % h->fat->bytes_per_sector;
	block_read_sectors(h->fat->dev, sector, 1, sector_buf); 

	for(;; offset += 32) {
		/* Read next sector if needed */
		if(offset == h->fat->bytes_per_sector) {
			sector++;
			if(!h->root_flag && 
			   ((sector % h->fat->sectors_per_cluster) == 0)) {
				/* Go to next cluster... */
				h->cur_cluster = fat_get_next_cluster(h->fat, 
							h->cur_cluster);
				if(h->cur_cluster == fat_eoc(h->fat)) 
					return 0;
				sector = fat_first_sector_of_cluster(h->fat, 
							h->cur_cluster);
			}
			block_read_sectors(h->fat->dev, sector, 1, sector_buf);
			offset = 0;
		}
		memcpy(&ent->fatent, sector_buf + offset, 32);
		h->position += 32;

		if(ent->fatent.name[0] == 0) 
			return 0;	/* Empty entry, end of directory */
		if(ent->fatent.name[0] == (char)0xe5)
			continue;	/* Deleted entry */
		if(ent->fatent.attr == FAT_ATTR_LONG_NAME) {
#ifdef LONG_NAME_SUPPORT
			struct fat_ldirent *ld = (void*)&ent->fatent;
			if(ld->ord & FAT_LAST_LONG_ENTRY) {
				memset(ent->d_name, 0, sizeof(ent->d_name));
				csum = ld->checksum;
			} 
			if(csum != ld->checksum) /* Abandon orphaned entry */
				csum = -1;

			i = ((ld->ord & 0x3f) - 1) * 13;

			/* If entries can't be converted to ASCII, abandon
			 * the long filename.  DOS 8.3 name will be returned.
			 * Not pretty... */
			switch(ascii_from_utf16(&ent->d_name[i], ld->name1, 5))
				{ case 0: continue; case 2: csum = -1; }
			switch(ascii_from_utf16(&ent->d_name[i+5], ld->name2, 6))
				{ case 0: continue; case 2: csum = -1; }
			switch(ascii_from_utf16(&ent->d_name[i+11], ld->name3, 2))
				{ case 0: continue; case 2: csum = -1; }
#endif
			continue;
		}
#ifdef LONG_NAME_SUPPORT
		if(csum != lfn_chksum((uint8_t*)ent->fatent.name)) 
			ent->d_name[0] = 0;

		if(ent->d_name[0] == 0) {
#endif
			for(i = 0, j = 0; i < 11; i++, j++) {
				ent->d_name[j] = tolower(ent->fatent.name[i]);
				if(ent->fatent.name[i] == ' ') {
					ent->d_name[j] = '.';
					while((ent->fatent.name[++i] == ' ') && (i < 11));
				}
			} 
			if(ent->d_name[j-1] == '.')
				ent->d_name[j-1] = 0;

			ent->d_name[j] = 0;
#ifdef LONG_NAME_SUPPORT
		}
#endif

		return 1;
	}
	return 0;
}

int fat_comparesfn(const char * name, const char *fatname)
{
	char canonname[11];
	int i;
	memset(canonname, ' ', sizeof(canonname));
	if(name[0] == '.') {
		/* Special case:
		 * Only legal names are '.' and '..' */
		memcpy(canonname, name, strlen(name));
	} else for(i = 0; (i < 11) && *name && (*name != '/'); i++) {
		if(*name == '.') {
			if(i < 8) continue;
			if(i == 8) name++;
		}       
		canonname[i] = toupper(*name++);
	}
	return ((*name == 0) || (*name == '/')) && !memcmp(canonname, fatname, 11);
}

int fat_dir_open_file(const struct fat_file_handle *dir, const char *name,
		struct fat_file_handle *file)
{
#ifdef LONG_NAME_SUPPORT
	uint16_t csum = -1;
#endif
	uint32_t cluster = 0;
	uint32_t sector;
	uint16_t offset = 0;

	/* Fail if dir isn't a directory */
	if(!dir->root_flag && dir->size)
		return 0;

	if(dir->root_flag) {
		/* FAT12/FAT16 root directory */
		sector = dir->first_cluster;
	} else {
		cluster = dir->first_cluster;
		sector = fat_first_sector_of_cluster(dir->fat, cluster);
	}
	block_read_sectors(dir->fat->dev, sector, 1, sector_buf); 

	for(;; offset += 32) {
		/* Read next sector if needed */
		if(offset == dir->fat->bytes_per_sector) {
			sector++;
			if(!dir->root_flag && 
			   ((sector % dir->fat->sectors_per_cluster) == 0)) {
				/* Go to next cluster... */
				cluster = fat_get_next_cluster(dir->fat, 
							cluster);
				/* End of cluster chain: file not found */
				if(cluster == fat_eoc(dir->fat)) 
					return 0;
				sector = fat_first_sector_of_cluster(dir->fat, 
							cluster);
			}
			block_read_sectors(dir->fat->dev, sector, 1, sector_buf);
			offset = 0;
		}

		struct fat_dirent *fatent = (void*)&sector_buf[offset];

		if(fatent->name[0] == 0) 
			return 0;	/* Empty entry, end of directory */
		if(fatent->name[0] == (char)0xe5)
			continue;	/* Deleted entry */
		if(fatent->attr == FAT_ATTR_LONG_NAME) {
#ifdef LONG_NAME_SUPPORT
			struct fat_ldirent *ld = (void*)fatent;
			int i;

			if(ld->ord & FAT_LAST_LONG_ENTRY)
				csum = ld->checksum;
			if(csum != ld->checksum) /* Abandon orphaned entry */
				csum = -1;

			i = ((ld->ord & 0x3f) - 1) * 13;

			/* Compare LFN to name, trash csum if mismatch */
			switch(ascii_cmp_utf16(&name[i], ld->name1, 5))
				{ case 0: continue; case 2: csum = -1; }
			switch(ascii_cmp_utf16(&name[i+5], ld->name2, 6))
				{ case 0: continue; case 2: csum = -1; }
			switch(ascii_cmp_utf16(&name[i+11], ld->name3, 2))
				{ case 0: continue; case 2: csum = -1; }
#endif
			continue;
		}
#ifdef LONG_NAME_SUPPORT
		/* Long name match */
		if(csum == lfn_chksum((uint8_t*)fatent->name)) {
			fat_file_init(dir->fat, fatent, file);
			return 1;
		}
#endif
		/* Check for short name match */
		if(fat_comparesfn(name, fatent->name)) {
			fat_file_init(dir->fat, fatent, file);
			return 1;
		}
	}
	return 0;
}

