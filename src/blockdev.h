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

/* Block device abstraction.  
 * Must be implemented by application for a physical device.
 */

#ifndef __BLOCKDEV_H
#define __BLOCKDEV_H

#include <stdint.h>

struct block_device {
	/* Info about the device */
	uint16_t (*get_sector_size)(const struct block_device *dev);
	/* ... more to be added as needed ... */

	/* Actions on the device */
	int (*read_sectors)(const struct block_device *dev, 
			uint32_t sector, uint32_t count, void *buf);
	/* ... more to be added as needed ... */

	/* May be private fields here ... */
};

/* Convenient wrapper functions */
static inline uint16_t
block_get_sector_size(const struct block_device *dev)
{
	return dev->get_sector_size(dev);
}

static inline int 
block_read_sectors(const struct block_device *dev,
		uint32_t sector, uint32_t count, void *buf)
{
	return dev->read_sectors(dev, sector, count, buf);
}

#endif

