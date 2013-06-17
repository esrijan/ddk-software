/*
 * Copyright (C) eSrijan Innovations Private Limited
 * 
 * Author: Anil Kumar Pugalia <anil_pugalia@eSrijan.com>
 *
 * Licensed under: JSL (See LICENSE file for details)
 *
 * ATmega16/32
 *
 * Serial loopback over IR, using bit banging of only the data bits (when
 * BIT_BANGING is defined) or using the RS232 protocol, otherwise
 *
 * For trying this example on DDK's, make sure of the following:
 * 	DDK v1.1: RX jumper should be on the left-most pin pair.
 * 	DDK v2.1: Both RX-TX jumpers should be on the right-most pin pairs.
 */

#include <avr/io.h>
#include <util/delay.h>

#include "serial.h"

//#define BIT_BANGING
/*
 * >= 4000bps == 250su: doesn't work
 * 2400bps == 417us: works for really no sunlight noise
 * <= 2000bps == 500us: works for minimal sunlight noise
 */

void init_serial(void)
{
#ifndef BIT_BANGING
	usart_init(1200);
#endif

	DDRB |= (1 << 7);
#ifdef BIT_BANGING
	DDRD = 0b00000010;
#endif
}

int main(void)
{
	unsigned char t_c, r_c;
#ifdef BIT_BANGING
	int i;
#endif

	init_serial();

	t_c = '0';
	while (1)
	{
#ifdef BIT_BANGING
		r_c = 0;
		for (i = 0; i < 8; i++)
		{
			if (t_c & (1 << i))
			{
				PORTD |= (1 << 1);
			}
			else
			{
				PORTD &= ~(1 << 1);
			}
			_delay_us(250);
			if (PIND & (1 << 0))
			{
				r_c |= (1 << i);
			}
			else
			{
				r_c &= ~(1 << i);
			}
		}
#else
		usart_byte_tx(t_c);
		_delay_ms(5);
		r_c = usart_byte_rx();
#endif
		if (r_c == t_c)
		{
			PORTB |= (1 << 7);
		}
		else
		{
			PORTB &= ~(1 << 7);
		}
		if (t_c++ == '9')
			t_c = '0';
		_delay_ms(5);
	}

	return 0;
}
