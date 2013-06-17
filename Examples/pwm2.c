/*
 * Copyright (C) eSrijan Innovations Private Limited
 * 
 * Author: Anil Kumar Pugalia <anil_pugalia@eSrijan.com>
 *
 * Licensed under: JSL (See LICENSE file for details)
 *
 * ATmega16/32
 * 
 * Keep on generating square wave of particular frequency - Testable using
 * multimeter f mode
 */

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

void set_scale(uint8_t scale)
{
	OCR2 = scale; // f = (1/2) * (F_CPU / PRESCALING) / (1 + OCR2) = 31.25KHz / (1 + scale)
}
void init_pwm(uint8_t scale)
{
	DDRD |= (1 << PD7); // Switch to o/p, basically connecting the OC2 out

	set_scale(scale);

	// Setup OC2 on PD7
	TCCR2 = (1 << WGM21) | (0 << WGM20); /* Clear Timer on Compare Match */
	TCCR2 |= (1 << COM20); /* Toggle on Match */
	TCCR2 |= (6 << CS20); /* Prescaling /256 => Clock @ F_CPU/256 */ // Starts the PWM
}

int main(void)
{
	uint8_t scale = 0;

	init_pwm(scale);

	while (1)
	{
		_delay_ms(1000);
		set_scale(++scale);
	}

	return 0;
}
