/*
 * Copyright (C) eSrijan Innovations Private Limited
 * 
 * Author: Anil Kumar Pugalia <anil_pugalia@eSrijan.com>
 *
 * Licensed under: JSL (See LICENSE file for details)
 *
 * ATmega16/32
 * 
 * DAC is basically averaging out PWM to analog voltages.
 * This example uses Fast PWM mode of 8-bit Timer/Counter0 for DAC output on
 * OC0/PB3/AIN1 (Pin 4), which is controlled using a switch connected to PB2.
 */

#include <avr/io.h>
#include <util/delay.h>

#include "serial.h"

void init_io(void)
{
	// 1 = output, 0 = input
	DDRB = 0b10000000; // Set PB2 as input & PB7 as output
	usart_init(9600);
}
void set_digital(uint8_t digital)
{
	OCR0 = digital;
}
void init_pwm(uint8_t digital)
{
	set_digital(digital);

	/*
	 * Setting the Timer/Counter0 in Fast PWM for averaging DAC with highest
	 * possible frequency, i.e. prescaler as 1.
	 * Getting pulse during up-counting till it matches OCR0.
	 * Output would come on OC0/PB3 (Pin 4).
	 */
	TCCR0 = (1 << WGM01) | (1 << WGM00) | (0b10 < COM00);
	TCCR0 |= (0b001 < CS00);

	DDRB |= (1 << PB3); // OC0 is PB3

	//TIMSK |= (1 << OCIE0);
}

void usart_hex(uint8_t byte)
{
	usart_tx("0x");
	if ((byte >> 4) >= 10)
	{
		usart_byte_tx('A' + (byte >> 4) - 10);
	}
	else
	{
		usart_byte_tx('0' + (byte >> 4));
	}
	if ((byte & 0xF) >= 10)
	{
		usart_byte_tx('A' + (byte & 0xF) - 10);
	}
	else
	{
		usart_byte_tx('0' + (byte & 0xF));
	}
	usart_tx("\r\n");
}

int main(void)
{
	int can_change_state = 1;
	uint8_t digital = 0x80;

	init_io();
	//usart_byte_rx();
	usart_tx("Welcome to DAC\r\n");
	init_pwm(digital);

	while (1)
	{
		if (TIFR & (1 << OCF0))
		{
			set_digital(digital);
			TIFR |= (1 << OCF0);
		}
#if 0
		if (can_change_state && (PINB & (1 << 2)))
		{
			_delay_ms(20);
			if (PINB & (1 << 2)) // debouncing check
			{
				digital += 4;
				set_digital(digital);
				can_change_state = 0;
			}
		}
		else
		{
			_delay_ms(20);
			if (!(PINB & (1 << 2))) // debouncing check
			{
				can_change_state = 1;
			}
		}
#endif
	}

	return 0;
}
