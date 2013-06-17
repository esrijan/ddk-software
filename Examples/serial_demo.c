/*
 * Copyright (C) eSrijan Innovations Private Limited
 * 
 * Author: Anil Kumar Pugalia <anil_pugalia@eSrijan.com>
 *
 * Licensed under: JSL (See LICENSE file for details)
 *
 * ATmega16/32
 *
 * Serial Communication to translate character to its ascii in hex.
 *
 * Using LED at PB7 for debugging.
 *
 * For trying this example on DDK's, make sure of the following:
 * 	DDK v1.1: RX jumper should be on the left-most pin pair.
 * 	DDK v2.1: Both RX-TX jumpers should be on the right-most pin pairs.
 */

#include <avr/io.h>

#include "serial.h"

void init_serial(void)
{
	usart_init(38400);

	DDRB |= (1 << 7);
}
void char_to_hex(char c, char a[3])
{
	a[0] = (c >> 4) & 0x0F;
	a[1] = c & 0x0F;
	a[2] = 0;
	a[0] += (a[0] < 10) ? '0' : ('A' - 10);
	a[1] += (a[1] < 10) ? '0' : ('A' - 10);
}

int main(void)
{
	char c;
	char ascii[3];

	init_serial();

	usart_byte_rx(); // Waiting for a character, typically an <Enter>
	usart_tx("Welcome to eSrijan's ASCII translator\r\n");

	while (1)
	{
		usart_tx("eSrijan> ");
		c = usart_byte_rx();
		PORTB ^= (1 << 7);
		usart_tx("You gave: ");
		usart_byte_tx(c);
		usart_tx(" (");
		char_to_hex(c, ascii);
		usart_tx(ascii);
		usart_tx(")\r\n");
	}

	return 0;
}
