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
 * Timer interrupt handler triggers at a 1KHz frequency, i.e. every 1ms, and does the following every 10th trigger,
 * i.e. at a frequency of 100Hz:
 * + Receives a bit from PC0 connected to ASK Receiver (Long PCB)'s digital output. On receiving a complete byte, it
 * 	is displayed onto the serial based on the PROMISCUS & DEBUG defines. All bytes when DEBUG is defined; all
 * 	non-default bytes only, when only PROMISCUS is defined; All packetized data only, otherwise.
 * + Transmits a bit of user received data to ASK Transmitter (Small PCB)'s input connected to PC1
 * Before this whole rx-tx transmission start, calibration is done to sync w/ the byte start, if CALI is defined.
 */

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "serial.h"

#define NUM_TO_ASCII(n) (((n) < 10) ? ('0' + (n)) : ('A' + (n) - 10))
#define ASCII_TO_NUM(a) (((a) <= '9') ? ((a) - '0') : ((a) - 'A' + 10))

#if (F_CPU == 16000000)
#define PRESCALAR_BITS 0b011
#define PRESCALAR 64 // Gives 250KHz clock
#else
#error Non-supported clock frequency
#endif

#undef DEBUG
#define CALI // Required till clock funda is decided upon
#define PROMISCUS // Define to get the raw data

#ifdef DEBUG
#undef BIT_SHIFT
#endif
#ifdef CALI
#define CALI_DATA 0xEC
#endif
/* Its either of the one below as per the initial state of 0 or 1 */
#define DEF_DATA ((uint8_t)(0x00))
#define DEF_INV_DATA ((uint8_t)(~0x00))
#define IN_DATA (PINC & (1 << PC0)) // (!(PINC & (1 << PC0))) /* Invert in case of using a Schmidt trigger */
#define OUT_PIN (1 << PC1)
/* Packet defines */
#define PKT_START 0xBE
#define PKT_DUMMY 0xBF
#define PKT_END 0xED

#ifdef CALI
int cali = 0;
#endif
char ready = 0;
uint8_t data_out[16];
uint8_t data_len;

void usart_byte_tx_hex(uint8_t data);
void recv_data(void);
void send_data(void);

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
			data_len = ind;
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
ISR(TIMER0_COMP_vect)
{
	static int cnt = 0;

	if (++cnt == 10)
	{
		cnt = 0;
		recv_data();
		send_data();
	}
}

void init_timer(void) /* Setting Timer 0 for trigger every 1ms */
{
	TIMSK |= (1 << OCIE0); // Enable Compare Match interrupt
	/*
	 * Pre-scaled clock = F_CPU / PRESCALAR
	 * => Each timer counter increment takes PRESCALAR / F_CPU seconds
	 * => Formula for timer expiry interval is OCR0 (top timer count) * PRESCALAR / F_CPU
	 * Example: For F_CPU = 16MHz, PRESCALAR = 64
	 * Pre-scaled clock = 16MHz / 64 = 250KHz
	 * => Each timer counter increment takes 1/250KHz = 4us
	 * => Formula for timer expiry interval is OCR0 (top timer count) * 4us
	 * Example: For 1ms = OCR0 * 4us, i.e. OCR0 = 250
	 */
	OCR0 = (F_CPU / PRESCALAR) / 1000; /* 1/1000 for 1ms */
	/*
	 * Setting & Starting the Timer/Counter0 in CTC (Clear Timer on Compare) (non-PWM) for
	 * controlling timer expiry interval, directly by the compare register &
	 */
	TCCR0 = (1 << WGM01) | (0 << WGM00) | PRESCALAR_BITS;
}
void init_device(void)
{
	DDRC |= 0b10;

	usart_init(9600);

	sei(); // Enable global interrupts
	UCSRB |= (1 << RXCIE); // Enable receive complete interrupt

	init_timer();
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
	static uint8_t data_in = 0;
#ifdef PROMISCUS
#ifdef BIT_SHIFT
	static uint8_t prev_bit = 0;
#endif
#else
	static int inside_pkt = 0, pkt_len = 0, pkt_recv_len = 0;
#endif

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
		{
			usart_byte_tx_hex(data_in);
			usart_tx("\nN>");
		}
#endif
		return;
	}
#endif
	if (rx_ind-- == 0)
	{
#ifdef PROMISCUS
#ifdef CALI
		if (data_in != CALI_DATA)
#endif
		if ((data_in != DEF_DATA) && (data_in != DEF_INV_DATA))
		{
			usart_tx("\nD: ");
			usart_byte_tx(data_in);
			usart_byte_tx_hex(data_in);
#ifdef BIT_SHIFT
			usart_byte_tx_hex((prev_bit << 7) | (data_in >> 1));
#endif
			usart_tx("\nR>");
		}
#ifdef BIT_SHIFT
		prev_bit = data_in & 0x1;
#endif
#else
		if (inside_pkt == 0)
		{
			if (data_in == PKT_START)
			{
				inside_pkt = 1;
			}
#ifdef DEBUG
			else
			{
				usart_byte_tx_hex(data_in);
				usart_tx("\nJ>");
			}
#endif
		}
		else if (inside_pkt == 1)
		{
			usart_tx("\nL: ");
			usart_byte_tx(data_in);
#ifdef DEBUG
			usart_byte_tx_hex(data_in);
#endif
			pkt_len = ASCII_TO_NUM(data_in) << 4;
			inside_pkt = 2;
		}
		else if (inside_pkt == 2)
		{
			usart_byte_tx(data_in);
#ifdef DEBUG
			usart_byte_tx_hex(data_in);
#endif
			pkt_len |= ASCII_TO_NUM(data_in);
			inside_pkt = 3;
			pkt_recv_len = 0;
			usart_tx("\nP: ");
		}
		else if (inside_pkt == 3)
		{
			if (pkt_recv_len++ < pkt_len)
			{
				usart_byte_tx(data_in);
				//usart_byte_tx_hex(data_in);
			}
			else
			{
				if (data_in != PKT_END)
				{
					usart_tx("\nPacket end not detected");
#ifdef DEBUG
					usart_tx(": ");
					usart_byte_tx_hex(data_in);
#endif
					usart_tx("\n");
				}
				inside_pkt = 0;
				usart_tx("\nR>");
			}
		}
#ifdef DEBUG
		else // Should not happen
		{
			usart_byte_tx_hex(data_in);
			usart_tx("\nI>");
		}
#endif
#endif
		rx_ind = 7;
		data_in = 0;
	}
}
void send_data(void)
{
	static int ind = -4;
	static int tx_ind = 7;
	static int clk_i = 7;
	static uint8_t data;

	if (ready && (clk_i == 7))
	{
		if (tx_ind == 7)
		{
			if (ind == -4)
			{
				data = PKT_DUMMY;
			}
			if (ind == -3)
			{
				data = PKT_START;
			}
			else if (ind == -2)
			{
				data = NUM_TO_ASCII(data_len >> 4);
			}
			else if (ind == -1)
			{
				data = NUM_TO_ASCII(data_len & 0xF);
			}
			else if (ind < data_len)
			{
				data = data_out[ind];
			}
			else if (ind == data_len)
			{
				data = PKT_END;
			}
			else
			{
				data = PKT_DUMMY;
			}
		}
		if (data & (1 << tx_ind))
			PORTC |= OUT_PIN;
		else
			PORTC &= ~OUT_PIN;
		if (tx_ind-- == 0)
		{
			tx_ind = 7;
			if (ind == (data_len + 1)) // As 2 bytes after packet data
			{
				ind = -4;
				ready = 0;
			}
			else
				ind++;
		}
	}
	else
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
	init_device();
	//DDRB |= (1 << 7);

	usart_tx("F>");
	while (1)
	{
		//PORTB ^= (1 << 7);
		_delay_ms(500);
	}

	return 0;
}
