/*
 * Copyright (C) eSrijan Innovations Private Limited
 * 
 * Author: Anil Kumar Pugalia <anil_pugalia@eSrijan.com>
 *
 * Licensed under: JSL (See LICENSE file for details)
 *
 * ATmega48/88/168, ATmega16/32
 * 
 * Storing & Reading data into program space (flash)
 * Demonstrated by toggling PB7 pin at 1Hz - can be observed on an LED
 */

#include <avr/io.h>
#include <avr/pgmspace.h>
#include <util/delay.h>

uint8_t ssd_map[1] PROGMEM = { 0xFF };

int main(void)
{
	uint8_t map;

	// All outputs
	DDRB = 0b10000000;
	map = pgm_read_word(ssd_map);
	while (1)
	{
		PORTB = 0x00;
		_delay_ms(500);
		PORTB = map;
		_delay_ms(500);
	}
	return 0;
}
