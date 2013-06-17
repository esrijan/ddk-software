/*
 * Copyright (C) eSrijan Innovations Private Limited
 * 
 * Author: Anil Kumar Pugalia <anil_pugalia@eSrijan.com>
 *
 * Licensed under: JSL (See LICENSE file for details)
 *
 * ATmega16/32
 * 
 * On detection of the switch press (on PB2), transmits IR controlled through PC7
 * Then, read the IR input from PC0, and outputs the same to led (on PB7)
 */

#include <avr/io.h>
#include <util/delay.h>

#include "ddk.h"

void init_io(void)
{
	// 1 = output, 0 = input
	BUTTON_INIT;
	DDRB |= 0b10000000;
	DDRC |= 0b10000000;
}

int main(void)
{
	init_io();

	while (1)
	{
		if (BUTTON_PRESSED)
		{
			PORTC |= (1 << 7);
		}
		else
		{
			PORTC &= ~(1 << 7);
		}
		if (PINC & (1 << 0))
		{
			PORTB |= (1 << 7);
		}
		else
		{
			PORTB &= ~(1 << 7);
		}
		_delay_ms(20);
	}

	return 0;
}
