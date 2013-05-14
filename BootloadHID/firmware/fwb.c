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

int flash_write_block(uint8_t *block_addr, uint8_t *data)
{
#if (FLASHEND) > 0xFFFF /* we need long addressing */
	unsigned long a = (unsigned long)(block_addr);
#else
	unsigned int a = (unsigned int)(block_addr);
#endif
	int i;

	if ((a & (BLOCK_SIZE - 1)) != 0) /* Not block size aligned */
		return -1;

	if (a >= BL_ADDR) /* Allowing to write only in the application section */
		return -1;

	cli(); // Disable interruptions

	/* Erase the (flash) block */
	boot_page_erase(a);
	boot_spm_busy_wait(); /* Wait until page is erased */

	/* Start filling the block's buffer word-wise */
	for (i = 0; i < BLOCK_SIZE; i += 2)
	{
		boot_page_fill(a + i, *(uint16_t *)(data + i));
	}

	/* Write the block's buffer into the flash */
	boot_page_write(a);
	boot_spm_busy_wait(); /* Wait until page is written */

	boot_rww_enable(); /* Re-enable RWW-section again */
	/* The above is needed to access the flash written */

	sei(); // Enable interrupts

	return 0;
}
