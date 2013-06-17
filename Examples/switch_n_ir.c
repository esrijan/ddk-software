/*
 * Copyright (C) eSrijan Innovations Private Limited
 * 
 * Author: Anil Kumar Pugalia <anil_pugalia@eSrijan.com>
 *
 * Licensed under: JSL (See LICENSE file for details)
 *
 * ATmega48/88/168, ATmega16/32
 * 
 * Sends out IR (through IR LED connected on PB3) on switch (PB2) press.
 * Alongwith toggle the LED on PB7.
 */

#include <avr/io.h>
#include <util/delay.h>

#include "ddk.h"

void init_io(void)
{
	// 1 = output, 0 = input
	BUTTON_INIT;
	DDRB = 0b10001000;
}

int main(void)
{
	init_io();

	while (1)
	{
		if (BUTTON_PRESSED)
		{
			PORTB |= (1 << PB3);
			PORTB |= (1 << PB7);
		}
		else
		{
			PORTB &= ~(1 << PB3);
			PORTB &= ~(1 << PB7);
		}
	}

	return 0;
}
