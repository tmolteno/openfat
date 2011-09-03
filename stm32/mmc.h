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

/* MMC Card interface.
 */

#ifndef __MMC_H
#define __MMC_H

#include <stdint.h>

#include "blockdev.h"

struct mmc_port {
	struct block_device bldev;
	/* Physical hardware config */
	uint32_t spi;
	uint32_t cs_port;
	uint16_t cs_pin;
	/* Any state information */
	/* ... to be added ... */
};

/* MMC command mnemonics in SPI mode */
#define MMC_GO_IDLE_STATE 	0
#define MMC_SEND_OP_COND	1
#define MMC_SWITCH		6
#define MMC_SEND_EXT_CSD	8
#define MMC_SEND_CSD		9
#define MMC_SEND_CID		10
#define MMC_STOP_TRANSMISSION	12
#define MMC_SEND_STATUS		13
#define MMC_SET_BLOCKLEN	16
#define MMC_READ_SINGLE_BLOCK	17
#define MMC_READ_MULTIPLE_BLOCK	18
#define MMC_SET_BLOCK_COUNT	23
#define MMC_WRITE_BLOCK		24
#define MMC_WRITE_MULTIPLE_BLOCK	25
#define MMC_PROGRAM_CSD		27
#define MMC_SET_WRITE_PROT	28
#define MMC_CLR_WRITE_PROT	29
#define MMC_SEND_WRITE_PROT	30
#define MMC_ERASE_GROUP_START	35
#define MMC_ERASE_GROUP_END	36
#define MMC_ERASE		38
#define MMC_LOCK_UNLOCK		42
#define MMC_APP_CMD		55
#define MMC_GEN_CMD		56
#define MMC_READ_OCR		58
#define MMC_CRC_ON_OFF		59

int mmc_init(uint32_t spi, uint32_t cs_port, uint16_t cs_pin, struct mmc_port *mmc);

#endif

