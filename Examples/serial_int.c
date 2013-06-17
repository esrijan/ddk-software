/*
 * Copyright (C) eSrijan Innovations Private Limited
 * 
 * Author: Anil Kumar Pugalia <anil_pugalia@eSrijan.com>
 *
 * Licensed under: JSL (See LICENSE file for details)
 *
 * ATmega16/32
 *
 * Interrupt based Serial Communication to echo back the character, and
 * set PB3 to LSB of character received
 *
 * Using LED at PB7 for debugging
 *
 * For trying this example on DDK's, make sure of the following:
 * 	DDK v1.1: RX jumper should be on the left-most pin pair.
 * 	DDK v2.1: Both RX-TX jumpers should be on the right-most pin pairs.
 */

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "serial.h"

ISR(USART_RXC_vect)
/*
 * This routine should read the character received to clear the RXC flag.
 * Otherwise, this interrupt would be triggered again on exit of this routine.
 */
{
	char c;

	c = usart_byte_rx();
	if (c & 0b1)
	{
		PORTB |= (1 << 3) | (1 << 7);
	}
	else
	{
		PORTB &= ~((1 << 3) | (1 << 7));
	}
	usart_byte_tx(c);
}

void init_io(void)
{
	PORTB &= ~((1 << 3) | (1 << 7));
	DDRB |= (1 << 3) | (1 << 7);

	usart_init(9600);

	sei(); // Enable global interrupts
	UCSRB |= (1 << RXCIE); // Enable receive complete interrupt
}

int main(void)
{
	init_io();

	while (1)
	{
		_delay_ms(1000);
	}

	return 0;
}
