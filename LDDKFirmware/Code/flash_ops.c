/*
 * Copyright (C) eSrijan Innovations Private Limited
 * 
 * Author: Anil Kumar Pugalia <anil_pugalia@eSrijan.com>
 *
 * Licensed under: JSL (See LICENSE file for details)
 *
 * ATmega16/32
 * 
 * Flash Operation Functions
 */

#include <avr/io.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include <avr/boot.h>

#include "flash_ops.h"

uint16_t flash_read_word(uint16_t *addr)
{
	return pgm_read_word(addr);
}
void flash_write_word(uint16_t *addr, uint16_t word)
{
#if (FLASHEND) > 0xFFFF /* we need long addressing */
	unsigned long a = (unsigned long)(addr);
#else
	unsigned int a = (unsigned int)(addr);
#endif

	/* Erase page, when we are at page's start */
	if ((a & (SPM_PAGESIZE - 1)) == 0)
	{
		cli();
		boot_page_erase(a);
		sei();
		boot_spm_busy_wait(); /* Wait until page is erased */
	}
	cli();
	boot_page_fill(a, word);
	sei();
	/* Write page when we are at page's last word */
	if ((a & (SPM_PAGESIZE - 1)) == (SPM_PAGESIZE - 2))
	{
		cli();
		boot_page_write(a & ~(SPM_PAGESIZE - 1));
		sei();
		boot_spm_busy_wait();
	}
}
