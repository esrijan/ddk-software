/*
 * Copyright (C) eSrijan Innovations Private Limited
 * 
 * Author: Anil Kumar Pugalia <anil_pugalia@eSrijan.com>
 *
 * Licensed under: JSL (See LICENSE file for details)
 *
 * ATmega16/32
 *
 * ASK RF based communication demo w/ serial port console. 
 * Serial interrupt handler receives characters from user input and stores them for transmission.
 * The main loop operating at a 100Hz frequency by CLOCK_DELAY, does the following on every iteration:
 * + Receives a bit from PC0 connected to ASK Receiver (Long PCB)'s digital output. On receiving a complete byte, it
 * 	is displayed onto the serial
 * + Transmits a bit of user received data to ASK Transmitter (Small PCB)'s input connected to PC1
 * Before this whole rx-tx transmission commencement, calibration is done to sync w/ the byte start.
 *
 * For trying this example on DDK's, make sure of the following:
 * 	DDK v1.1: RX jumper should be on the left-most pin pair.
 * 	DDK v2.1: Both RX-TX jumpers should be on the right-most pin pairs.
 */

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "serial.h"

#undef DEBUG
#define CALI // Required till clock funda is decided upon

#ifdef CALI
#define CALI_DATA 0xEC
#endif
/* Its either of the one below as per the initial state of 0 or 1 */
#define DEF_DATA ((uint8_t)(0x00))
#define DEF_INV_DATA ((uint8_t)(~0x00))
#define IN_DATA (PINC & (1 << PC0)) // (!(PINC & (1 << PC0))) /* Invert in case of using a Schmidt trigger */
#define OUT_PIN (1 << 1)
#define CLOCK_DELAY _delay_ms(10) /* 100 Hz === 1 / 100s === 10ms */

#ifdef CALI
int cali = 0;
#endif
char ready = 0;
uint8_t data_out[16];

void usart_byte_tx_hex(uint8_t data);

ISR(USART_RXC_vect)
/*
 * This routine should read the character received to clear the RXC flag.
 * Otherwise, this interrupt would be triggered again on exit of this routine.
 */
{
	uint8_t c;
	static int ind = 0;

	if (ready) // Previous data not yet consumed
		return;

	c = usart_byte_rx();
#ifdef DEBUG
	usart_tx("S:");
	usart_byte_tx_hex(c);
	usart_tx("\n");
#endif
	if (c == '\n')
	{
		data_out[ind] = 0;
		if (ind > 0)
		{
			ready = 1;
			ind = 0;
#ifdef DEBUG
			usart_tx((char *)data_out);
#endif
			usart_byte_tx('$');
		}
		usart_byte_tx('>');
	}
	else if (ind < 16)
	{
		data_out[ind++] = c;
	}
}

void init_io(void)
{
	DDRC |= 0b10;
}
void init_serial(void)
{
	usart_init(9600);

	sei(); // Enable global interrupts
	UCSRB |= (1 << RXCIE); // Enable receive complete interrupt
}

void usart_byte_tx_hex(uint8_t data)
{
	uint8_t nibble;

	nibble = data >> 4;
	usart_byte_tx(nibble < 10 ? '0' + nibble : 'A' + nibble - 10);
	nibble = data & 0xF;
	usart_byte_tx(nibble < 10 ? '0' + nibble : 'A' + nibble - 10);
}
void recv_data(void)
{
	static int rx_ind = 7;
#ifdef DEBUG
	static uint8_t prev_bit = 0;
#endif
	static uint8_t data_in = 0;

	data_in = (data_in << 1) | IN_DATA;
#ifdef CALI
	if (!cali)
	{
		if (data_in == CALI_DATA)
		{
			cali = 1;
			usart_tx("\nCalibrated with ");
			usart_byte_tx_hex(data_in);
			usart_tx("\nC>");
		}
#ifdef DEBUG
		else
			usart_byte_tx_hex(data_in);
#endif
		return;
	}
#endif
	if (rx_ind-- == 0)
	{
#ifdef CALI
		if (data_in != CALI_DATA)
#endif
		if ((data_in != DEF_DATA) && (data_in != DEF_INV_DATA))
		{
			usart_tx("\nD: ");
			usart_byte_tx(data_in);
			usart_byte_tx_hex(data_in);
#ifdef DEBUG
			usart_byte_tx_hex((prev_bit << 7) | (data_in >> 1));
#endif
			usart_tx("\nR>");
		}
		rx_ind = 7;
#ifdef DEBUG
		prev_bit = data_in & 0x1;
#endif
		data_in = 0;
	}
}
void send_data(void)
{
	static int ind = 0;
	static int tx_ind = 7;
	static int clk_i = 7;

	if (ready && (clk_i == 7))
	{
		if (data_out[ind])
		{
			if (data_out[ind] & (1 << tx_ind))
				PORTC |= OUT_PIN;
			else
				PORTC &= ~OUT_PIN;
			if (tx_ind-- == 0)
			{
				ind++;
				tx_ind = 7;
			}
		}
		else
		{
			ready = 0;
			ind = 0;
		}
	}
	if (!ready || (clk_i != 7))
	{
#ifdef CALI
		if (!cali)
		{
			if (CALI_DATA & (1 << clk_i))
				PORTC |= OUT_PIN;
			else
				PORTC &= ~OUT_PIN;
		}
#endif
		if (clk_i-- == 0)
			clk_i = 7;
	}
}

int main(void)
{
	init_io();
	init_serial();

	usart_tx("F>");
	while (1)
	{
		recv_data();
		send_data();
		CLOCK_DELAY;
	}

	return 0;
}
