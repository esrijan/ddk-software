/*
 * Copyright (C) eSrijan Innovations Private Limited
 * 
 * Author: Anil Kumar Pugalia <anil_pugalia@eSrijan.com>
 *
 * Licensed under: JSL (See LICENSE file for details)
 *
 * ATmega16/32
 *
 * ASK RF based communication across 2 independent devices: a transmitter & a receiver. 
 * On receiver (i.e. when ENABLE_RX is defined):
 * + Timer interrupt handler triggers at a 1KHz frequency, i.e. every 1ms, and does the following every trigger,
 * 	> Receives a bit from PC0 connected to ASK Receiver (Long PCB)'s digital output. On receiving a complete byte, it
 * 	is displayed onto the serial based on the PROMISCUS & DEBUG defines. All bytes when DEBUG is defined; all
 * 	non-default bytes only, when only PROMISCUS is defined; All packetized data only, otherwise.
 * On transmitter (i.e. when ENABLE_TX is defined):
 * + Serial interrupt handler receives characters from user input and stores them for transmission.
 * + Timer interrupt handler triggers at a 1KHz frequency, i.e. every 1ms, and does the following every trigger,
 * 	> Transmits a bit of user received data to ASK Transmitter (Small PCB)'s input connected to PC1
 * (NB The data ready for transmission could achieved on receiving '\n' on serial or by BUTTON press, by defining
 * READY_BY_SWITCH)
 * Before this whole rx-tx transmission start, calibration is done to sync w/ the byte start, if CALI is defined.
 *
 * For trying this example on DDK's, make sure of the following:
 * 	DDK v1.1: RX jumper should be on the left-most pin pair.
 * 	DDK v2.1: Both RX-TX jumpers should be on the right-most pin pairs.
 */

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "ddk.h"
#include "serial.h"

#define NUM_TO_ASCII(n) (((n) < 10) ? ('0' + (n)) : ('A' + (n) - 10))
#define ASCII_TO_NUM(a) (((a) <= '9') ? ((a) - '0') : ((a) - 'A' + 10))

#if (F_CPU == 16000000)
#define PRESCALAR_BITS 0b011
#define PRESCALAR 64 // Gives 250KHz clock
#else
#error Non-supported clock frequency
#endif

#define ENABLE_RX
#undef ENABLE_TX

#define DEBUG 1 /* Allowed levels: 0, 1, 2 */
#define CALI // Required till clock funda is decided upon

#ifdef ENABLE_RX
#undef PROMISCUS // Define to get the raw data
#if (DEBUG >= 1)
#undef BIT_SHIFT
#endif
#endif
#ifdef ENABLE_TX
#define READY_BY_SWITCH
#endif

#ifdef CALI
#define CALI_DATA 0xEC
#endif
/* Its either of the one below as per the initial state of 0 or 1 */
#define DEF_DATA ((uint8_t)(0x00))
#define DEF_INV_DATA ((uint8_t)(~0x00))
#define IN_DATA (PINC & (1 << PC0)) // (!(PINC & (1 << PC0))) /* Invert in case of using a Schmidt trigger */
#define OUT_PIN (1 << 1)
/* Packet defines */
#define PKT_SYNC ((uint8_t)(0xAA)) /* Alternate 1's & 0's */
#define PKT_DUMMY ((uint8_t)(0x3C)) /* MSB should be 0 to distinguish from PKT_DUMMY */
#define PKT_START ((uint8_t)(0xBE))
#define PKT_END ((uint8_t)(0xED))
#define PKT_DATA_SIZE 16
#define PKT_SIZE (sizeof(union _u))

#ifdef CALI
int cali = 0;
#endif
#ifdef ENABLE_TX
int ready = 0;
union _u
{
	struct _pkt
	{
		uint8_t sync1;
		uint8_t sync2;
		uint8_t dummy1;
		uint8_t start;
		uint8_t len1;
		uint8_t len2;
		uint8_t data[PKT_DATA_SIZE];
		uint8_t end;
		uint8_t dummy2;
	} pkt;
	uint8_t stream[sizeof(struct _pkt)];
} snd = {{PKT_SYNC, PKT_SYNC, PKT_DUMMY, PKT_START, '0', '9', "123456789", PKT_END, PKT_DUMMY}}; // TODO
//} snd = {{PKT_SYNC, PKT_SYNC, PKT_DUMMY, PKT_START, '0', 'F', "123456789ABCDEF", PKT_END, PKT_DUMMY}};
#endif

void usart_byte_tx_hex(uint8_t data);
#ifdef ENABLE_RX
void recv_data(void);
#endif
#ifdef ENABLE_TX
void send_data(void);
void send_sync(void);
#endif

#ifdef ENABLE_TX
#ifndef READY_BY_SWITCH // TODO
ISR(USART_RXC_vect)
/*
 * This routine should read the character received to clear the RXC flag.
 * Otherwise, this interrupt would be triggered again on exit of this routine.
 */
{
	uint8_t c;
	static uint8_t ind = 0;

	if (ready) // Previous data not yet consumed
		return;

	c = usart_byte_rx();
#if (DEBUG >= 1)
	usart_tx("S:");
	usart_byte_tx_hex(c);
	usart_tx("\n");
#endif
	if (c == '\n')
	{
		snd.pkt.data[ind] = 0;
		if (ind > 0)
		{
			snd.pkt.len1 = NUM_TO_ASCII(ind >> 4);
			snd.pkt.len2 = NUM_TO_ASCII(ind & 0xF);
#ifndef READY_BY_SWITCH
			ready = 1;
#endif
			ind = 0;
#if (DEBUG >= 1)
			usart_tx((char *)snd.pkt.data);
#endif
			usart_byte_tx('$');
		}
		usart_byte_tx('>');
	}
	else if (ind < (PKT_DATA_SIZE - 1))
	{
		snd.pkt.data[ind++] = c;
#ifdef READY_BY_SWITCH
		snd.pkt.data[ind] = 0;
#endif
	}
}
#endif
#endif
ISR(TIMER0_COMP_vect)
{
#ifdef ENABLE_RX
		recv_data();
#endif
#ifdef ENABLE_TX
		send_data();
		//send_sync();
#endif
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
#ifdef ENABLE_RX
	DDRC &= ~(1 << PC0);
#endif
#ifdef ENABLE_TX
	DDRC |= (1 << PC1);
	BUTTON_INIT;
	DDRB &= ~(1 << PB2);
#endif

#if (defined(ENABLE_RX) || (defined(ENABLE_TX) && !defined(READY_BY_SWITCH)))
	usart_init(9600);
#endif

	sei(); // Enable global interrupts
#ifdef ENABLE_TX
#ifndef READY_BY_SWITCH
	UCSRB |= (1 << RXCIE); // Enable receive complete interrupt
#endif
#endif

	_delay_us(100);
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
#ifdef ENABLE_RX
void recv_data(void)
{
	static int rx_ind = 7;
	static uint8_t data_in = 0;
#ifdef PROMISCUS
#ifdef BIT_SHIFT
	static uint8_t prev_bit = 0;
#endif
#else
	static int is_synced = 0, sync_triggered = 0, inside_pkt = 0, pkt_len = 0, pkt_recv_len = 0;
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
#if (DEBUG >= 1)
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
		data_in = 0;
		rx_ind = 7;
#else
		if (!is_synced)
		{
			if (!sync_triggered)
			{
				if (data_in == PKT_SYNC)
				// PKT_SYNC started
				{
					sync_triggered = 1;
#if (DEBUG >= 1)
					usart_tx("\nS>");
#endif
					rx_ind += 2; // Get two more bits & re-try
				}
				else
				{
#if (DEBUG >= 2)
					usart_byte_tx_hex(data_in);
					usart_tx("\nJ>");
#endif
					rx_ind++; // Get one more bit & re-try
				}
			}
			else
			{
				if (data_in == PKT_SYNC)
				// PKT_SYNC continuing
				{
#if (DEBUG >= 1)
					usart_byte_tx('s');
#endif
					// Sync is continuing - get more bits
					rx_ind += 2; // Get two more bits & re-try
				}
				else if (data_in == (uint8_t)((PKT_SYNC << 2) | (PKT_DUMMY >> 6)))
				// Possibly PKT_DUMMY has started - get 6 more bits to complete it
				{
#if (DEBUG >= 1)
					usart_byte_tx('p');
#endif
					rx_ind = 5;
				}
				else if (data_in == PKT_DUMMY) // PKT_DUMMY received
				{
#if (DEBUG >= 1)
					usart_byte_tx('d');
#endif
					is_synced = 1;
					data_in = 0;
					rx_ind = 7;
				}
				else // Something went wrong - so restart
				{
#if (DEBUG >= 1)
					usart_byte_tx_hex(data_in);
					usart_tx("\nQ>");
#endif
					sync_triggered = 0;
					data_in = 0;
					rx_ind = 7;
				}
			}
			//data_in = 0; // Do not discard data, i.e. do not set data_in to 0
		}
		else
		{
			if (inside_pkt == 0)
			{
				if (data_in == PKT_START)
				{
#if (DEBUG >= 1)
					usart_byte_tx('b');
#endif
					inside_pkt = 1;
				}
				else
				{
#if (DEBUG >= 1)
					usart_byte_tx_hex(data_in);
					usart_tx("\nQ>");
#endif
					sync_triggered = 0;
					is_synced = 0;
				}
			}
			else if (inside_pkt == 1)
			{
				usart_tx("\nL: ");
				usart_byte_tx(data_in);
#if (DEBUG >= 1)
				usart_byte_tx_hex(data_in);
#endif
				pkt_len = ASCII_TO_NUM(data_in) << 4;
				inside_pkt = 2;
			}
			else if (inside_pkt == 2)
			{
				usart_byte_tx(data_in);
#if (DEBUG >= 1)
				usart_byte_tx_hex(data_in);
#endif
				pkt_len |= ASCII_TO_NUM(data_in);
				inside_pkt = 3;
				pkt_recv_len = 0;
#if (DEBUG >= 1)
				usart_byte_tx_hex((uint16_t)(pkt_len) >> 8);
				usart_byte_tx_hex((uint16_t)(pkt_len) & 0xFF);
#endif
				usart_tx("\nP: ");
			}
			else if (inside_pkt == 3)
			{
				if (pkt_recv_len < pkt_len)
				{
					usart_byte_tx(data_in);
#if (DEBUG >= 1)
					usart_byte_tx_hex(data_in);
#endif
					pkt_recv_len++;
				}
				else if (pkt_recv_len < PKT_DATA_SIZE)
				{
#if (DEBUG >= 1)
					usart_byte_tx_hex(data_in);
#endif
					pkt_recv_len++;
				}
				else
				{
					if (data_in != PKT_END)
					{
						usart_tx("\nPacket end not detected");
#if (DEBUG >= 1)
						usart_tx(": ");
						usart_byte_tx_hex(data_in);
#endif
						usart_tx("\n");
					}
					inside_pkt = 0;
					sync_triggered = 0;
					is_synced = 0;
					usart_tx("\nR>");
				}
			}
#if (DEBUG >= 1)
			else // Should not happen
			{
				usart_byte_tx_hex(data_in);
				usart_tx("\nI>");
			}
#endif
			data_in = 0;
			rx_ind = 7;
		}
#endif
	}
}
#endif
#ifdef ENABLE_TX
void send_data(void)
{
	static int ind = 0;
	static int tx_ind = 7;
	static int clk_i = 7;
	static uint8_t data;

	if (ready && (clk_i == 7))
	{
		if (tx_ind == 7)
		{
			data = snd.stream[ind];
#if (DEBUG >= 1)
			usart_byte_tx_hex(data);
#endif
		}
		if (data & (1 << tx_ind))
			PORTC |= OUT_PIN;
		else
			PORTC &= ~OUT_PIN;
		if (tx_ind-- == 0)
		{
			tx_ind = 7;
			if (ind == (PKT_SIZE - 1))
			{
				ind = 0;
				ready = 0;
#if (DEBUG >= 1)
				usart_tx("\nT>");
#endif
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
void send_sync(void)
{
	static int tx_ind = 0;

	if (tx_ind)
		PORTC |= OUT_PIN;
	else
		PORTC &= ~OUT_PIN;
	tx_ind = !tx_ind;
}
#endif

int main(void)
{
#ifdef ENABLE_TX
#ifdef READY_BY_SWITCH
	int may_be_ready;
#endif
#endif

	init_device();

#ifdef ENABLE_TX
#ifdef READY_BY_SWITCH
	may_be_ready = 0;
#endif
#endif
#ifdef ENABLE_RX
	usart_tx("Fr>");
#endif
#ifdef ENABLE_TX
	usart_tx("Ft>");
#endif
	while (1)
	{
#ifdef ENABLE_TX
#if (defined(CALI) || defined(READY_BY_SWITCH))
		if (BUTTON_PRESSED)
		{
#ifdef CALI
			if (!cali)
			{
				cali = 1;
				usart_tx("\nOut of calibration\nO>");
				_delay_ms(20); // debouncing time
			}
#endif
#ifdef READY_BY_SWITCH
			may_be_ready = 1;
#endif
		}
#endif
#endif
		_delay_ms(20); // debouncing time
#ifdef ENABLE_TX
#ifdef READY_BY_SWITCH
		if (may_be_ready && (BUTTON_PRESSED))
		{
			may_be_ready = 0;
			ready = 1;
#if (DEBUG >= 1)
			usart_tx("r");
#endif
		}
#endif
#endif
	}

	return 0;
}
