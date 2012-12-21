/*
 * Copyright (C) eSrijan Innovations Private Limited
 * 
 * Author: Anil Kumar Pugalia <anil_pugalia@eSrijan.com>
 *
 * Licensed under: JSL (See LICENSE file for details)
 *
 * ATmega48/88/168, ATmega16/32
 * 
 * Example Blink. Toggles all IO pins at 1Hz
 */

#include <avr/io.h>
#include <util/delay.h>

void init_io(void)
{
	// 1 = output, 0 = input
	DDRB = 0b11111111; // All outputs
	DDRC = 0b11111111; // All outputs
	DDRD = 0b11111110; // PORTD (RX on PD0). Just for demo
}

int main(void)
{
	init_io();

	while (1)
	{
		PORTB = 0xFF;
		PORTC = 0xFF;
		PORTD = 0xFF;
		_delay_ms(500);

		PORTB = 0x00;
		PORTC = 0x00;
		PORTD = 0x00;
		_delay_ms(500);
	}

	return 0;
}
