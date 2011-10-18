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

#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/spi.h>
#include <libopencm3/stm32/systick.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>

#include <assert.h>

#include "openfat.h"
#include "openfat/mbr.h"

#include "mmc.h"

void stm32_setup(void)
{
	/* Setup SYSCLK */
	rcc_clock_setup_in_hsi_out_48mhz();

	/* Enable peripheral clocks for GPIOA, GPIOB, SPI2 */
	rcc_peripheral_enable_clock(&RCC_APB2ENR, 
				RCC_APB2ENR_IOPAEN | RCC_APB2ENR_IOPBEN);

	rcc_peripheral_enable_clock(&RCC_APB1ENR, RCC_APB1ENR_SPI2EN);

	/* Enable power to SD card */
	gpio_set_mode(GPIOA, GPIO_MODE_OUTPUT_10_MHZ, GPIO_CNF_OUTPUT_PUSHPULL,
			GPIO2);
	gpio_set(GPIOA, GPIO2);

	/* Force to SPI mode.  This should be default after reset! */
	SPI2_I2SCFGR = 0; 

	/* Configure SD card i/f SPI2: PB13(SCK), PB14(MISO), PB15(MOSI) */
	gpio_set_mode(GPIOB, GPIO_MODE_OUTPUT_10_MHZ, 
			GPIO_CNF_OUTPUT_ALTFN_PUSHPULL,
			GPIO_SPI2_SCK | GPIO_SPI2_MOSI);
	gpio_set_mode(GPIOB, GPIO_MODE_INPUT, GPIO_CNF_INPUT_FLOAT,
			GPIO_SPI2_MISO);
	/* SD nCS pin is GPIOA 3 */
	gpio_set_mode(GPIOA, GPIO_MODE_OUTPUT_10_MHZ, GPIO_CNF_OUTPUT_PUSHPULL,
			GPIO3);

	/* Enable Systick for benchmark timing */
	/* 48MHz / 8 => 6000000 counts per second */
	systick_set_clocksource(STK_CTRL_CLKSOURCE_AHB_DIV8); 
	/* 6000000/6000 = 1000 overflows per second - every 1ms one interrupt */
	systick_set_reload(6000);
	systick_interrupt_enable();
	/* start counting */
	systick_counter_enable();

	/* Enable LED output */
	gpio_set_mode(GPIOB, GPIO_MODE_OUTPUT_10_MHZ, GPIO_CNF_OUTPUT_PUSHPULL,
			GPIO12);
	gpio_set(GPIOB, GPIO12);
}

void print_tree(struct fat_vol_handle *vol, struct fat_file_handle *dir, int nest)
{
	struct fat_file_handle subdir;
	struct dirent ent;

	while(!fat_readdir(dir, &ent)) {
		if((strcmp(ent.d_name, ".") == 0) || 
		   (strcmp(ent.d_name, "..") == 0))
			continue;

		for(int i = 0; i < nest; i++) printf("\t");
		printf("%s\n", ent.d_name);

		if(ent.fat_attr == FAT_ATTR_DIRECTORY) {
			fat_chdir(vol, ent.d_name);
			assert(fat_open(vol, ".", 0, &subdir) == 0);
			print_tree(vol, &subdir, nest + 1);
			fat_chdir(vol, "..");
		}
	}

}

int _write(int fd, char *buf, int len)
{
	return len;
}

int main(void)
{
	struct mmc_port spi2;
	struct block_mbr_partition part;
	struct block_device *bldev = (void*)&spi2;
	struct block_device *blpart = (void*)&part;
	struct fat_vol_handle fat;
	struct fat_file_handle root, file;

	stm32_setup();

	mmc_init(SPI2, GPIOA, GPIO3, &spi2);
	mbr_partition_init(&part, bldev, 0);

	assert(fat_vol_init(blpart, &fat) == 0);
	printf("Fat type is FAT%d\n", fat.type);

	fat_mkdir(&fat, "Directory1");
	assert(fat_chdir(&fat, "Directory1") == 0);
	assert(fat_open(&fat, "Message file with a long name.txt", O_CREAT, &file) == 0);
	for(int i = 0; i < 100; i++) {
		char message[80];
		sprintf(message, "Here is a message %d\n", i);
		fat_write(&file, message, strlen(message));
	}
	assert(fat_chdir(&fat, "..") == 0);

	assert(fat_open(&fat, ".", 0, &root) == 0);
	print_tree(&fat, &root, 0);

	while (1) {
	}

	return 0;
}

void sys_tick_handler()
{
	static int temp32;

	temp32++;

	/* we call this handler every 1ms so 1000ms = 1s on/off */
	if (temp32 == 1000) {
		gpio_toggle(GPIOB, GPIO12); /* LED2 on/off */
		temp32 = 0;
	}
}


