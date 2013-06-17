/*
 * Copyright (C) eSrijan Innovations Private Limited
 * 
 * Author: Anil Kumar Pugalia <anil_pugalia@eSrijan.com>
 *
 * Licensed under: JSL (See LICENSE file for details)
 *
 * ATmega16/32
 * 
 * Emit various frequency notes onto the buzzer using a switch.
 * Buzzer is assumed to be connected between OC2/PD7 (Pin 21) & GND.
 * Frequency generation is done using CTC of 8-bit Timer/Counter2.
 * Switch is connected to PB2.
 */

#include <avr/io.h>
#include <util/delay.h>

#include "ddk.h"

//#define MANUAL
#define HUMAN_AUDIBLE_MAX_FREQ 20000

/*
 * The following has been decided based on the possible_freq.txt and by
 * actually hearing the audio
 */
#define PRESCALER (6 << CS20)
#define MAX_FREQ ((F_CPU / 256) / 2)

void init_io(void)
{
	// 1 = output, 0 = input
	BUTTON_INIT;
	DDRB &= ~(1 << PB2); // Set PB2 as input
}
void set_frequency(unsigned long freq)
/* f = MAX_FREQ / (1 + OCR2), i.e. OCR2 = MAX_FREQ / f - 1; */
{
	if (freq < (MAX_FREQ / 256))
	{
		freq = (MAX_FREQ / 256);
	}
	else if (freq > MAX_FREQ)
	{
		freq = MAX_FREQ;
	}
	OCR2 = MAX_FREQ / freq - 1;
}
void init_timer(unsigned long freq)
{
	DDRD |= (1 << PD7); // OC2 is PD7

	set_frequency(freq);

	/*
	 * Setting the Timer/Counter2 in CTC (Clear Timer on Compare) (non-PWM) for
	 * controlling frequency of waveforms, directly by the compare register &
	 * Toggling on Match to generate square wave for a particular frequency.
	 * Output would come on OC2/PD7 (Pin 21).
	 */
	TCCR2 = (1 << WGM21) | (0 << WGM20) | (1 << COM20) | PRESCALER;
}

int main(void)
{
#ifdef MANUAL
	int can_change_state = 1;
#endif
	unsigned long freq = MAX_FREQ / 256;

	init_io();
	init_timer(freq);

	while (1)
	{
#ifdef MANUAL
		if (can_change_state && (BUTTON_PRESSED))
		{
			_delay_ms(20);
			if (BUTTON_PRESSED) // debouncing check
			{
				freq += (MAX_FREQ / 256);
				if (freq > HUMAN_AUDIBLE_MAX_FREQ)
				{
					freq = MAX_FREQ / 256;
				}
				set_frequency(freq);
				can_change_state = 0;
			}
		}
		else
		{
			_delay_ms(20);
			if (BUTTON_RELEASED) // debouncing check
			{
				can_change_state = 1;
			}
		}
#else
		_delay_ms(100);
		freq += (MAX_FREQ / 256);
		if (freq > HUMAN_AUDIBLE_MAX_FREQ)
		{
			freq = MAX_FREQ / 256;
		}
		set_frequency(freq);
#endif
	}

	return 0;
}
