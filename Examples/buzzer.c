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

/*
 * Human audible frequency range is 20Hz to 20KHz. So, setting to generate
 * roughly upto 20KHz, which means a minimum of 20KHz / 256 ~ 80Hz
 */
#if (F_CPU < 20000)
#error Proecessor frequency should be between 20 KHz & 200 MHz
#elif (F_CPU < 160000)
#define PRESCALAR (0b001 << CS20) // Prescaler is /1
#define MAX_FREQ F_CPU
#elif (F_CPU < 640000)
#define PRESCALAR (0b010 << CS20) // Prescaler is /8
#define MAX_FREQ (F_CPU / 8)
#elif (F_CPU < 1280000)
#define PRESCALAR (0b011 << CS20) // Prescaler is /32
#define MAX_FREQ (F_CPU / 32)
#elif (F_CPU < 2560000)
#define PRESCALAR (0b100 << CS20) // Prescaler is /64
#define MAX_FREQ (F_CPU / 64)
#elif (F_CPU < 5120000)
#define PRESCALAR (0b101 << CS20) // Prescaler is /128
#define MAX_FREQ (F_CPU / 128)
#elif (F_CPU < 20480000)
#define PRESCALAR (0b110 << CS20) // Prescaler is /256
#define MAX_FREQ (F_CPU / 256)
#elif (F_CPU <= 200000000) // Max f which could generate a min. of ~800Hz
#define PRESCALAR (0b111 << CS20) // Prescaler is /1024
#define MAX_FREQ (F_CPU / 1024)
#else
#error Proecessor frequency should be between 20 KHz & 200 MHz
#endif

void init_io(void)
{
	// 1 = output, 0 = input
	DDRB &= ~(1 << PB2); // Set PB2 as input
}
void set_frequency(unsigned freq)
{
	if (freq < (MAX_FREQ / 256))
	{
		freq = (MAX_FREQ / 256);
	}
	else if (freq > MAX_FREQ)
	{
		freq = MAX_FREQ;
	}
	OCR2 = MAX_FREQ / freq;
}
void init_timer(unsigned freq)
{
	set_frequency(freq);

	/*
	 * Setting the Timer/Counter2 in CTC (Clear Timer on Compare) (non-PWM) for
	 * controlling frequency of waveforms, directly by the compare register &
	 * Toggling on Match to generate square wave for a particular frequency.
	 * Output would come on OC2/PD7 (Pin 21).
	 */
	TCCR2 = (1 << WGM21) | (0 << WGM20) | (0b01 < COM20) | PRESCALAR;

	DDRD |= (1 << PD7); // OC2 is PD7
}

int main(void)
{
	int can_change_state = 1;
	unsigned freq = MAX_FREQ / 256;

	init_io();
	init_timer(freq);

	while (1)
	{
		if (can_change_state && (PINB & (1 << PB2)))
		{
			_delay_ms(20);
			if (PINB & (1 << 2)) // debouncing check
			{
				freq++;
				set_frequency(freq);
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
	}

	return 0;
}
