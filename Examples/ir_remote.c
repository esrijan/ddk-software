/*
 * Copyright (C) eSrijan Innovations Private Limited
 * 
 * Author: Anil Kumar Pugalia <anil_pugalia@eSrijan.com>
 *
 * Licensed under: JSL (See LICENSE file for details)
 *
 * ATmega16/32
 * 
 * On switch press (from PB2), sends out a remote key code, modulated over 38KHz
 * or 40KHz (depending on the define CF_38KHZ), from OC2/PD7, using PWM on
 * Timer 2.
 *
 * LED (on PB7) is used for debugging
 */

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "ddk.h"

#define CF_38KHZ

#define ARRAY_SIZE(a) (sizeof(a) / sizeof(*a))
/*
 * Wait time to re-check for the processing getting over (for the next
 * transmission values). Should be less than carrier time period, say half of
 * it - to be on the safer side
 */
#define RECHECK_WAIT_DELAY _delay_us(13) // (1/2)*(1/38KHz)

enum
{
	off,
	on
};

volatile uint8_t not_processed;
uint16_t cycles_to_load;
uint8_t carrier_state_to_load;

static void carrier_on(void)
{
	DDRD |= (1 << DDD7); // Switch to o/p, basically connecting the OC2 out
}
static void carrier_off(void)
{
	PORTD &= ~(1 << PD7); // Prepare pull down
	DDRD &= ~(1 << DDD7); // Switch to i/p, basically cutting off the OC2 out
}
ISR(TIMER2_COMP_vect)
{
	static uint8_t half_cycle = 0;
	static uint16_t cycles = 0;
	static uint8_t carrier_state = off;

	half_cycle = !half_cycle;
	if (!half_cycle) // beginning of a fresh cycle;
	{
		if (cycles) // previous transmission going on - continue as is
		{
			cycles--;
		}
		else // no transmission in progress
		{
			if (not_processed) // reload for the next transmission
			{
				cycles = cycles_to_load;
				carrier_state = carrier_state_to_load;
				not_processed = 0;
				if (carrier_state == off)
				{
					carrier_off();
				}
				else
				{
					carrier_on();
				}
				cycles--;
			}
			else // nothing else to do - switch off
			{
				carrier_off();
			}
		}
	}
}

void init_pwm(void)
{
	carrier_off();

	not_processed = 0;
	cycles_to_load = 0;
	carrier_state_to_load = off;

	TIMSK |= (1 << OCIE2); // Generate interrupt every state change
	sei();

#ifdef CF_38KHZ
	OCR2 = 25; // f = (1/2) * (F_CPU / PRESCALING) / (1 + OCR2) = 1MHz / 26
#else
	OCR2 = 24; // f = (1/2) * (F_CPU / PRESCALING) / (1 + OCR2) = 1MHz / 25
#endif

	// Setup OC2 on PD7
	TCCR2 = (1 << WGM21) | (0 << WGM20); /* Clear Timer on Compare Match */
	TCCR2 |= (1 << COM20); /* Toggle on Match */
	TCCR2 |= (2 << CS20); /* Prescaling /8 => Clock @ F_CPU/8 */ // Starts the PWM
}
void init_io(void)
{
	init_pwm();

	// 1 = output, 0 = input
	BUTTON_INIT;
	DDRB |= (1 << DDB7);
}

static void set_carrier_on_cycles(uint16_t cycles)
{
	while (not_processed)
		RECHECK_WAIT_DELAY;
	cycles_to_load = cycles;
	carrier_state_to_load = on;
	not_processed = 1;
}
static void set_carrier_off_cycles(uint16_t cycles)
{
	while (not_processed)
		RECHECK_WAIT_DELAY;
	cycles_to_load = cycles;
	carrier_state_to_load = off;
	not_processed = 1;
}
static void burst_pair(uint16_t on_cycles, uint16_t off_cycles)
{
	set_carrier_on_cycles(on_cycles);
	set_carrier_off_cycles(off_cycles);
}

/*
 * Samsung TV: BN59-01003A; Closest: BN59-00940A
 *
 * Packet decoded from: daemons/hw_default.c -> default_send(r, c) -> ... ->
 * send_code(r, c, .) (daemons/transmit.c); And daemons/irrecord.c ->
 * fprint_remote_head for the config file contents interpretation & mapping
 * with the C variables.
 *
 * Carrier Frequency: 38KHz w/ duty cycle 50%
 *
 * x flags: SPACE_ENC|CONST_LENGTH
 * x eps: 30 (default)
 * x aeps: 100 (default)
 *
 * bits: 16
 * header: 4498p 4450s
 * plead :    x     x
 * pre:       x     x
 * pre_data_bits: 16
 * pre_data: 0xE0E0
 * one   :  588p 1646s
 * zero  :  588p  535s
 * post:      x     x
 * post_data_bits: x
 * post_data: x
 * ptrail:  584p
 * foot  :    x     x
 *
 * total bits: 16 + 16 + 0 = 32
 * repeat:    x     x
 * gap: 107635 x
 * toggle_bit_mask: 0x0
 * toggle_mask: x
 *
 * frequency: x
 * duty cycle: x
 *
 * Power: 0x40BF
 */
void send_logic_one(void)
{
	burst_pair(588, 1646);
}
void send_logic_zero(void)
{
	burst_pair(588, 535);
}
void send_data_bits(uint16_t data)
/* LSB to be sent out first */
{
	int i;

	for (i = 0; i < 16; i++)
	{
		if (data & (1 << i))
			send_logic_one();
		else
			send_logic_zero();
	}
}

void send_samsung_ir_code(uint16_t data)
{
	/* Header */
	burst_pair(4498, 4450);
	/* Lead */
	/* Pre */
	send_data_bits(0xE0E0);
	/* Data */
	send_data_bits(data);
	/* Post */
	/* Trail */
	set_carrier_on_cycles(584);
	/* Foot */
}
#define POWER 0x40BF

int main(void)
{
	init_io();

	while (1)
	{
		if (BUTTON_PRESSED)
		{
			PORTB |= (1 << PB7); // Ready to transmit
			send_samsung_ir_code(POWER); // Send out the IR key code
			PORTB &= ~(1 << PB7); // Transmission over
		}
#if 0
		else
		{
			PORTB &= ~(1 << PB7);
		}
#endif
		_delay_ms(20);
	}

	return 0;
}
