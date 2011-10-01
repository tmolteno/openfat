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

/* Sample block device implementation:
 * Implementation of abstract block device over a Unix file
 */

#define _FILE_OFFSET_BITS 64

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "openfat/blockdev.h"

#define FILE_SECTOR_SIZE 512

struct block_device_file {
	struct block_device bldev;
	FILE *file;
};

uint16_t file_get_sector_size(const struct block_device *dev)
{
	(void)dev;
	return FILE_SECTOR_SIZE;
}

static int file_read_sectors(const struct block_device *bldev,
		uint32_t sector, uint32_t count, void *buf)
{
	const struct block_device_file *dev = (void*)bldev;
 
	fseeko(dev->file, (uint64_t)sector * FILE_SECTOR_SIZE, SEEK_SET);
	return fread(buf, FILE_SECTOR_SIZE, count, dev->file);
}

static int file_write_sectors(const struct block_device *bldev,
		uint32_t sector, uint32_t count, const void *buf)
{
	const struct block_device_file *dev = (void*)bldev;
 
	fseeko(dev->file, (uint64_t)sector * FILE_SECTOR_SIZE, SEEK_SET);
	return fwrite(buf, FILE_SECTOR_SIZE, count, dev->file);
}

struct block_device * block_device_file_new(const char *filename, const char *mode)
{
	FILE *f;
	struct block_device_file *dev;
	struct block_device *bldev;

	f = fopen(filename, mode);
	if(f == NULL)
		return NULL; 

	dev = malloc(sizeof(*dev));
	bldev = (void*)dev;

	bldev->read_sectors = file_read_sectors;
	bldev->write_sectors = file_write_sectors;
	bldev->get_sector_size = file_get_sector_size;
	dev->file = f;

	return (struct block_device*)dev;
}

void block_device_file_destroy(struct block_device *bldev)
{
	struct block_device_file *dev = (void*)bldev;

	fclose(dev->file);
	free(dev);
}

