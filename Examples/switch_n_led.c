/*
 * Copyright (C) eSrijan Innovations Private Limited
 * 
 * Author: Anil Kumar Pugalia <anil_pugalia@eSrijan.com>
 *
 * Licensed under: JSL (See LICENSE file for details)
 *
 * ATmega48/88/168, ATmega16/32
 * 
 * Reads switch input (active high) from PB2, and output the same to led (PB7)
 * Alternately, toggles PC0 with a delay of 0.5 sec
 * PC0 can be used as the CLOCK & PB7 as DATA for a shift register
 */

#include <avr/io.h>
#include <util/delay.h>

void init_io(void)
{
	// 1 = output, 0 = input
	DDRB |= 0b10000000;
	DDRC |= 0b00000001;
}

int main(void)
{
	init_io();

	while (1)
	{
		if (PINB & (1 << 2))
		{
			PORTB |= (1 << 7);
		}
		else
		{
			PORTB &= ~(1 << 7);
		}
		PORTC = 0xFF;
		_delay_ms(500);
		PORTC = 0xFE;
		_delay_ms(500);
	}

	return 0;
}
