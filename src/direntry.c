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
#include "fat.h"
#include "direntry.h"

uint8_t lfn_chksum(uint8_t *dosname)
{
	uint8_t sum = 0;
	int i;

	for (i = 0; i < 11; i++) 
		sum = ((sum & 1) ? 0x80 : 0) + (sum >> 1) + *dosname++;

	return sum;
}

int fat_dir_read(struct fat_file_handle *dir, struct dirent *ent)
{
	uint16_t csum = -1;
	int i, j;

	while(fat_file_read(dir, &ent->fatent, 
				sizeof(ent->fatent)) == sizeof(ent->fatent)) {

		if(ent->fatent.name[0] == 0) 
			return 0;	/* Empty entry, end of directory */
		if(ent->fatent.name[0] == (char)0xe5)
			continue;	/* Deleted entry */
		if(ent->fatent.attr == FAT_ATTR_LONG_NAME) {
			struct fat_ldirent *ld = (void*)&ent->fatent;
			if(ld->ord & FAT_LAST_LONG_ENTRY) {
				memset(ent->d_name, 0, sizeof(ent->d_name));
				csum = ld->checksum;
			} 
			if(csum != ld->checksum) 
				csum = -1;

			i = ((ld->ord & 0x3f) - 1) * 13;

			for(j = 0; j < 5; j++) 
				ent->d_name[i+j] = __get_le16(&ld->name1[j]);
			for(j = 0; j < 6; j++) 
				ent->d_name[i+5+j] = __get_le16(&ld->name2[j]);
			for(j = 0; j < 2; j++) 
				ent->d_name[i+11+j] = __get_le16(&ld->name3[j]);

			continue;
		}

		if(csum != lfn_chksum((uint8_t*)ent->fatent.name)) 
			ent->d_name[0] = 0;

		if(ent->d_name[0] == 0) {
			for(i = 0, j = 0; i < 11; i++, j++) {
				ent->d_name[j] = tolower(ent->fatent.name[i]);
				if(ent->fatent.name[i] == ' ') {
					ent->d_name[j++] = (ent->d_name[0] == '.') ? ' ' : '.';
					while((ent->fatent.name[++i] == ' ') && (i < 11));
				}
			} 
		}

		return 1;
	}
	return 0;
}

