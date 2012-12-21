/*
 * Copyright (C) eSrijan Innovations Private Limited
 * 
 * Author: Anil Kumar Pugalia <anil_pugalia@eSrijan.com>
 *
 * Licensed under: JSL (See LICENSE file for details)
 *
 * ATmega48/88/168
 * 
 * Controlling an RGB LED colours using PWM
 * Setup OC0A on PD6 (Pin 12) -> B of RGB LED
 * Setup OC0B on PD5 (Pin 11) -> G of RGB LED
 * Setup OC2B on PD3 (Pin 5) -> R of RGB LED
 * With fuses: L: E2; H: DF; E: 1 (Indicating an internal RC oscillator @ 8MHz)
 */

#include <avr/io.h>
#define NO_SYS_DELAY // TODO: Commenting this increases the executable hugely. Why?
#ifndef NO_SYS_DELAY
#include <util/delay.h>
#else
#include "common.h"
#define _delay_ms delay_ms
#endif

// TODO: How to get the following fuse values generated in the .hex file?
FUSES = 
{
	.low = LFUSE_DEFAULT | ~FUSE_CKDIV8,
	.high = HFUSE_DEFAULT,
	.extended = EFUSE_DEFAULT,
};

void set_rgb(uint8_t r, uint8_t g, uint8_t b)
{
	OCR0A = ~b;
	OCR0B = ~g;
	OCR2B = ~r;
}

void init_pwm(uint8_t r, uint8_t g, uint8_t b)
{
	set_rgb(r, g, b);

	// Setup OC0A on PD6 (Pin 12) - B
	// Setup OC0B on PD5 (Pin 11) - G
	TCCR0B = (1 << CS00);
	TCCR0A = (2 << COM0A0) | (3 << WGM00);
	TCCR0A |= (2 << COM0B0);
	// Setup OC2B on PD3 (Pin 5) - R
	TCCR2B = (1 << CS20);
	TCCR2A = (2 << COM2B0) | (3 << WGM20);

	DDRD = 0b01101000; // PD3,5,6
}

void zigzag_variation(int delay)
{
	uint8_t r = 0x7F, g = 0xFF, b = 0x00;
	uint8_t dir = 1;

	init_pwm(r, g, b);

	while (1)
	{
		_delay_ms(delay);
		if (b == 0)
		{
			dir = 1;
		}
		else if (b == 0xFF)
		{
			dir = -1;
		}
		r -= dir;
		g -= dir;
		b += dir;
		set_rgb(r, g, b);
	}
}

void circle_variation(int delay)
{
	uint8_t r = 0x00, g = 0x00, b = 0x00;

	init_pwm(r, g, b);
	//init_pwm(r >> 1, g, b); // For square RGB LED, r/2 as red is very intense

	while (1)
	{
		_delay_ms(delay);
		r++;
		g++;
		b++;
		set_rgb(r, g, b);
		//set_rgb(r >> 1, g, b); // For square RGB LED, r/2 as red is very intense
	}
}

void intensity_variation(uint8_t rr, uint8_t gr, uint8_t br, int delay)
{
	uint8_t multiplier;
	uint8_t r, g, b;
	
	multiplier = 0;
	r = (rr * multiplier) / 0xFF;
	g = (gr * multiplier) / 0xFF;
	b = (br * multiplier) / 0xFF;
	init_pwm(r, g, b);
	//init_pwm(r >> 1, g, b); // For square RGB LED, r/2 as red is very intense

	while (1)
	{
		_delay_ms(delay);
		multiplier++;
		r = (rr * multiplier) / 0xFF;
		g = (gr * multiplier) / 0xFF;
		b = (br * multiplier) / 0xFF;
		set_rgb(r, g, b);
		//set_rgb(r >> 1, g, b); // For square RGB LED, r/2 as red is very intense
	}
}

int main(void)
{
	//zigzag_variation(50);
	//circle_variation(50);
	//intensity_variation(0xFF, 0xFF, 0x00, 50);  // Orange -> Yellow
	intensity_variation(0xFF, 0x00, 0xFF, 50);  // Purple -> Pink

	return 0;
}
