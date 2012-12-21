/*
 * Copyright (C) eSrijan Innovations Private Limited
 * 
 * Author: Anil Kumar Pugalia <anil_pugalia@eSrijan.com>
 *
 * Licensed under: JSL (See LICENSE file for details)
 *
 * ATmega16/32
 * 
 * ASK RF based communication using RS232 protocol.
 * RF communication between RX433 & TX433 modules, tuned to operate at 433.92 MHz
 * DOUT (Pin 2) of RX433 is assumed to be connected to ATMega's RXD (PD0) - RX jumper pin @ 'RX'
 * DIN of TX433 is assumed to be connected to ATMega's TXD (PD1) - Hole 2 of DB9 connector.
 * Alongwith using switch input (active high) from PB2, and led output on (PB7).
 * And CLCD for display.
 */

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "clcd.h"
#include "serial.h"

#define SYNC 0xAA
#define RADD 0x44
#define L_ON 0x11
#define L_OF 0x22

#define PKT_ORIENTED 1

ISR(USART_RXC_vect)
/*
 * This routine should read the character received to clear the RXC flag.
 * Otherwise, this interrupt would be triggered again on exit of this routine.
 */
{
#ifdef PKT_ORIENTED
	uint8_t raddr, data, chksum;

	usart_byte_rx(); // Discard SYNC
	raddr = usart_byte_rx();
	data = usart_byte_rx();
	chksum = usart_byte_rx();
	if ((chksum == (raddr + data)) && (raddr == RADD))
	{
		clcd_move_to(16);
		clcd_print_string("Received: ");
		clcd_data_wr(data);
	}
	else // indicate error
	{
		if (('A' <= data) && (data <= 'Z'))
		{
			clcd_move_to(12);
			clcd_print_string("E: ");
			clcd_data_wr(data);
		}
	}
#else
	uint8_t in;

	in = usart_byte_rx();
	if (('A' <= in) && (in <= 'Z'))
	{
		clcd_move_to(16);
		clcd_print_string("Received: ");
		clcd_data_wr(in);
	}
#endif
}

void init_io(void)
{
	// 1 = output, 0 = input
	DDRA = 0b00000010; // PA1 as output & PA0 as input
	DDRB = 0b10000000; // PB7 as output & PB2 as input

	clcd_init();
	clcd_print_string("RF Duniya");
	clcd_move_to(0);
	usart_init(2400);
	sei(); // Enable global interrupts
	UCSRB |= (1 << RXCIE); // Enable receive complete interrupt
}

void rf_byte_tx(uint8_t byte)
{
	usart_byte_tx(byte);
}
int rf_byte_available(void)
{
	return usart_byte_available();
}
uint8_t rf_byte_rx(void)
{
	return usart_byte_rx();
}

void rf_pkt_tx(uint8_t addr, uint8_t cmd)
{
	rf_byte_tx(SYNC);
	rf_byte_tx(addr);
	rf_byte_tx(cmd);
	rf_byte_tx(addr + cmd); // Checksum
}

int main(void)
{
	int button_was_released = 0;
	uint8_t out = 'A';

	init_io();

	while (1)
	{
		/*
		if (!(PINB & (1 << 2))) // Switch released
		{
			button_was_released = 1;
		}
		else if (button_was_released && (PINB & (1 << 2))) // Switch pressed
		*/
		{
			button_was_released = 0;
			PORTB ^= (1 << 7); // Toggle LED
			clcd_move_to(0);
			clcd_print_string("Sending: ");
			clcd_data_wr(out);
#ifdef PKT_ORIENTED
			rf_pkt_tx(RADD, out);
#else
			rf_byte_tx(out);
#endif
			if (out++ == 'Z')
			{
				out = 'A';
			}
		}
		//_delay_ms(100);
		//_delay_ms(1000);
	}

	return 0;
}
