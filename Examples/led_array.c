/*
 * Copyright (C) eSrijan Innovations Private Limited
 * 
 * Author: Anil Kumar Pugalia <anil_pugalia@eSrijan.com>
 *
 * Licensed under: JSL (See LICENSE file for details)
 *
 * ATmega16/32
 * 
 * Generates different patterns on the 8 LED array (B7-B0) using the shift
 * register's Data (PD0) and Clk (PD1) on DDK v2.1.
 * Both RX-TX jumpers should be in centre pin pairs.
 * Note that the LEDs B7-B0 are all active low.
 * BUTTON (PB2) changes the speed of the pattern display.
 * DOWNLOAD (PB4) changes the pattern itslef.
 */

#include <avr/io.h>
#include <util/delay.h>

#include "ddk.h"

#define TOTAL_PATTERNS (sizeof(pattern) / sizeof(*pattern))

uint8_t pattern[] = {0xAA, 0xC3, 0x99};
uint8_t pattern_i = TOTAL_PATTERNS;
uint8_t delay = 5; /* In unit of 100ms */

void init_io(void)
{
	// 1 = output, 0 = input
	BUTTON_INIT;
	PORTB |= (0b00010000); // Pull-up on PB4
	DDRB |= 0b00000000;
	DDRD |= 0b00000011;
}

void clk(void) /* +ve edge */
{
	PORTD &= ~(1 << PD1);
	_delay_us(1);
	PORTD |= (1 << PD1);
	_delay_us(1);
}
void shift_byte(uint8_t data)
{
	int i;

	for (i = 7; i >= 0; i--)
	{
		if (data & (1 << i))
		{
			PORTD |= (1 << PD0);
		}
		else
		{
			PORTD &= ~(1 << PD0);
		}
		_delay_us(1);
		clk();
	}
}
void show_pattern(void)
{
	uint8_t pat;

	pat = (pattern_i != TOTAL_PATTERNS) ? pattern[pattern_i] : 0x00;
	shift_byte(pat);
	_delay_ms(100 * delay);
	pat = (pattern_i != TOTAL_PATTERNS) ? ~pattern[pattern_i] : 0x00;
	shift_byte(pat);
	_delay_ms(100 * delay);
}

int main(void)
{
	init_io();

	shift_byte(0x00); // Switch off the LED array to start with
	while (1)
	{
		if (!(PINB & (1 << PB4))) // DOWNLOAD pressed
		{
			if (pattern_i++ == TOTAL_PATTERNS)
				pattern_i = 0;
		}
		if (BUTTON_PRESSED) // BUTTON pressed
		{
			if (++delay == 15)
				delay = 0;
		}
		show_pattern();
	}

	return 0;
}
