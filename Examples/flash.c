/*
 * Copyright (C) eSrijan Innovations Private Limited
 * 
 * Author: Anil Kumar Pugalia <anil_pugalia@eSrijan.com>
 *
 * Licensed under: JSL (See LICENSE file for details)
 *
 * ATmega16/32
 *
 * Flash Operation Functions for DDK v2.1
 */

#include <avr/io.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include <avr/boot.h>

#include "flash.h"

uint8_t flash_read_byte(const uint8_t *addr)
{
	return pgm_read_byte(addr);
}
int flash_read_block(const uint8_t *block_addr, uint8_t *data)
{
#if (FLASHEND) > 0xFFFF /* we need long addressing */
	unsigned long a = (unsigned long)(block_addr);
#else
	unsigned int a = (unsigned int)(block_addr);
#endif
	int i;

	if ((a & (BLOCK_SIZE - 1)) != 0) /* Not block size aligned */
		return -1;

	for (i = 0; i < BLOCK_SIZE; i++)
	{
		data[i] = pgm_read_byte(block_addr + i);
	}
	return 0;
}
int (*flash_write_block)(uint8_t *block_addr, uint8_t *data) = (int (*)(uint8_t *, uint8_t *))(FWB_ADDR / 2);
