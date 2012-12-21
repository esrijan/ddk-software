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
 * NB Correct operation of USART module is expected only through a MAX232, i.e.
 * inverted, for both an original serial or USB2Serial (0-5V)
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
void usart_init(unsigned long baud)
{
	set_baud(baud);

	/* Default frame format: 8 data, No Parity, 1 stop bit (8N1) */
	//UCSRC = (1 < URSEL) | (3 << UCSZ0);
	/* Enable receiver and transmitter */
	UCSRB = (1 << RXEN) | (1 << TXEN);
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
	}
	str[i] = 0;
}
