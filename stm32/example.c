/*
 * This file is part of the openfatfs project.
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
}

int main(void)
{
	struct mmc_port spi2;

	stm32_setup();

	mmc_init(SPI2, GPIOA, GPIO3, &spi2);

	while (1) {
	}

	return 0;
}

