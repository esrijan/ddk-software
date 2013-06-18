/*
 * Copyright (C) eSrijan Innovations Private Limited
 * 
 * Author: Anil Kumar Pugalia <anil_pugalia@eSrijan.com>
 *
 * Licensed under: JSL (See LICENSE file for details)
 *
 * ATmega16/32
 * 
 * Serial Communication Functions
 * NB Correct operation of USART module w/ PC is expected only through a MAX232,
 * i.e. inverted, for both an original serial or USB2Serial (0-5V)
 *
 * For using this library on DDK's, make sure of the following:
 * 	DDK v1.1: RX jumper should be on the left-most pin pair.
 * 	DDK v2.1: Both RX-TX jumpers should be on the right-most pin pairs.
 */

#include <avr/io.h>

#include "serial.h"

static void set_baud(unsigned long baud)
/*
 * For default of Asynchronous mode and for USART0.
 * For Normal mode: baud = (F_CPU / (16 * (UBRR + 1)))
 * For Double spped mode: baud = (F_CPU / (8 * (UBRR + 1)))
 */
{
#ifndef COMPILE_TIME_BAUD_SET
	uint16_t ubrr;

#ifdef USE_2X
	ubrr = (F_CPU / 8 / baud) - 1;
#else
	ubrr = (F_CPU / 16 / baud) - 1;
#endif
	UBRRH = (uint8_t)(ubrr >> 8);
	UBRRL = (uint8_t)(ubrr);
#else
#define BAUD 9600
#include <util/setbaud.h>
	UBRRH = UBRRH_VALUE;
	UBRRL = UBRRL_VALUE;
#endif
#ifdef USE_2X
	UCSRA |= (1 << U2X);
#else
	UCSRA &= ~(1 << U2X);
#endif
}
static void set_format(uint8_t data_bits, Parity parity, uint8_t stop_bits)
{
	uint8_t control = (1 << URSEL);

	switch (data_bits)
	{
		case 5:
			control |= (0b00 << UCSZ0);
			// And, UCSRB &= ~(1 << UCSZ2);
			break;
		case 6:
			control |= (0b01 << UCSZ0);
			// And, UCSRB &= ~(1 << UCSZ2);
			break;
		case 7:
			control |= (0b10 << UCSZ0);
			// And, UCSRB &= ~(1 << UCSZ2);
			break;
		case 8:
		default:
			control |= (0b11 << UCSZ0);
			// And, UCSRB &= ~(1 << UCSZ2);
			break;
		case 9:
			control |= (0b11 << UCSZ0);
			// And, UCSRB |= (1 << UCSZ2);
			break;
	}
	switch (parity)
	{
		case p_none:
		default:
			control |= (0b00 << UPM0);
			break;
		case p_even:
			control |= (0b10 << UPM0);
			break;
		case p_odd:
			control |= (0b11 << UPM0);
			break;
	}
	switch (stop_bits)
	{
		case 1:
		default:
			control |= (0b0 << USBS);
			break;
		case 2:
			control |= (0b1 << USBS);
			break;
	}

	if (data_bits == 9)
		UCSRB |= (1 << UCSZ2);
	else
		UCSRB &= ~(1 << UCSZ2);
	UCSRC = control;
}

void usart_enable(void)
{
	/* Enable receiver and transmitter */
	UCSRB |= (1 << RXEN) | (1 << TXEN);
}
void usart_disable(void)
{
	/* Disable receiver and transmitter */
	UCSRB &= ~((1 << RXEN) | (1 << TXEN));
}

void usart_init(unsigned long baud)
{
	set_baud(baud);

	/* Default frame format: 8 data, No Parity, 1 stop bit (8N1) */
	set_format(8, p_none, 1);

	usart_enable();
}
void usart_shut(void)
{
	usart_disable();
}

void usart_byte_tx(uint8_t data)
{
	/* Wait for empty transmit buffer */
	while (!(UCSRA & (1 << UDRE)))
		;
	/* Put data into buffer, sends the data */
	UDR = data;
}
int usart_byte_available(void)
{
	return (UCSRA & (1 << RXC));
}
uint8_t usart_byte_rx(void)
{
	/* Wait for data to be received */
	while (!(UCSRA & (1 << RXC)))
		;
	/* Get and return received data from buffer */
	return UDR;
}
void usart_tx(char *str)
{
	while (*str)
	{
		usart_byte_tx(*str++);
	}
}
void usart_rx(char *str, int max_len)
{
	int i;

	for (i = 0; i < max_len - 1; i++)
	{
		str[i] = usart_byte_rx();
		if (str[i] == '\n')
			break;
	}
	str[i] = 0;
}
