/*
 * Copyright (C) eSrijan Innovations Private Limited
 * 
 * Author: Anil Kumar Pugalia <anil_pugalia@eSrijan.com>
 *
 * Licensed under: JSL (See LICENSE file for details)
 *
 * ATmega16/32
 *
 * Flash Write Block Function
 */

#include <avr/io.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include <avr/boot.h>

#include "fwb.h"

#ifdef FWB

#if (FLASHEND) > 0xFFFF /* we need long addressing */
int flash_write_block(uint32_t block_addr, uint8_t *data)
#else
int flash_write_block(uint16_t block_addr, uint8_t *data)
#endif
{
	int i;

	if ((block_addr & (BLOCK_SIZE - 1)) != 0) /* Not block size aligned */
		return -1;

	if (block_addr >= BL_ADDR) /* Allowing to write only in the application section */
		return -1;

	cli(); // Disable interruptions

	/* Erase the (flash) block */
	boot_page_erase(block_addr);
	boot_spm_busy_wait(); /* Wait until page is erased */

	/* Start filling the block's buffer word-wise */
	for (i = 0; i < BLOCK_SIZE; i += 2)
	{
		boot_page_fill(block_addr + i, *(uint16_t *)(data + i));
	}

	/* Write the block's buffer into the flash */
	boot_page_write(block_addr);
	boot_spm_busy_wait(); /* Wait until page is written */

	boot_rww_enable(); /* Re-enable RWW-section again */
	/* The above is needed to access the flash written */

	sei(); // Enable interrupts

	return 0;
}

#endif
