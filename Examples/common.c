/*
 * Copyright (C) eSrijan Innovations Private Limited
 * 
 * Author: Anil Kumar Pugalia <anil_pugalia@eSrijan.com>
 *
 * Licensed under: JSL (See LICENSE file for details)
 *
 * ATmega48/88/168, ATmega16/32
 *
 * Common Functions for ATmega48/88/168, ATmega16/32
 * 
 * Assumes definition of F_CPU as the processor frequency for delay_ms
 */

#include <avr/io.h>

#include "common.h"

void internal_clock_calibration(unsigned long freq)
{
	/*
	 * TODO: Not tested &/or properly written. Once done, possibly remove the
	 * following line
	 */
	(void)(freq);
	/*
	 * Fundae:
	 * Range = 7.3 MHz to 8.1 MHz
	 * OSCCAL = 0x00 to 0x7F & 0x80 to 0xFF
	 * Each OSCCAL unit = 0.8 MHz / 127 = 6299.2 Hz
	 * But, default OSCCAL is 0x66 (102) for achieving 8.0 MHz
	 * That gives each OSCCAL unit = (8.0 - 7.3) MHz / 102 = 6862.7 Hz
	 * Taking this, we would need to write a value of:
	 * (7.3728 - 7.3) MHz / 6862.7 Hz = 11 = 0xB
	 * TODO: But not sure of its working
	 */
	/* Calibration for making internal RC oscillator to 7.3728 MHz */
	OSCCAL = 11; // Assuming freq = 7372800
}

void delay_us(uint16_t usecs)
// As of tests, works best for F_CPU = 1MHz
{
	int8_t x;

	for (; usecs > 0; usecs--)
	{
		for (x = 0; x < (int8_t)((F_CPU / 1000000) - 2); x++) // -2 for compensating the above for loop
		{
			asm volatile ("nop");
		}
	}
}
void delay_ms(uint16_t msecs)
// As of tests, works best for F_CPU = 1MHz
{
	uint8_t x, y;

	for (; msecs > 0; msecs--)
	{
		for (x = 0; x < 90; x++)
		{
#if (F_CPU < 2000000) // Assembly is generated simple & matching up the requirement
			for (y = 0; y < 6 * (F_CPU / 1000000); y++)
#else // Double assembly is generated hence an extra div by 2
			for (y = 0; y < 3 * (F_CPU / 1000000); y++)
#endif
			{
				asm volatile ("nop");
			}
		}
	}
}
