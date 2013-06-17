/*
 * Copyright (C) eSrijan Innovations Private Limited
 * 
 * Author: Anil Kumar Pugalia <anil_pugalia@eSrijan.com>
 *
 * Licensed under: JSL (See LICENSE file for details)
 *
 * ATmega48/88/168, ATmega16/32
 * 
 * Toggles the LED connected at PB7 at 1Hz
 */

#include <avr/io.h>
#include <util/delay.h>

void init_io(void)
{
	// 1 = output, 0 = input
	DDRB |= 0b10000000;
}

int main(void)
{
	init_io();

	while (1)
	{
		PORTB |= (1 << 7);
		_delay_ms(500);
		PORTB &= ~(1 << 7);
		_delay_ms(500);
	}

	return 0;
}
