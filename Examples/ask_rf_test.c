/*
 * Copyright (C) eSrijan Innovations Private Limited
 * 
 * Author: Anil Kumar Pugalia <anil_pugalia@eSrijan.com>
 *
 * Licensed under: JSL (See LICENSE file for details)
 *
 * ATmega16/32
 * 
 * Generates various duty cycle square waves on PA0-5 @ 100Hz for input to ASK Transmitter (Small PCB).
 * Reads PA6-7 for linear & digital outputs from the ASK receiver (Longer PCB), respectively.
 * Then, puts the PA6-7 inputs received, on PC0 & PB7, respectively. PC0 is connected to B0 LED's input pin from
 * shift register, i.e. below the resistor. This shows the linear & digital values received on B0 & PGM leds,
 * respectively.
 * The timing is achieved using a delay loop running at a 10KHz frequency, i.e. every 100us.
 */

#include <avr/io.h>
#include <util/delay.h>

#undef ONE_KHZ

#define IPBIT0 (PINA & (1 << 6))
#define IPBIT1 (PINA & (1 << 7))
#define OPBIT0 (1 << 0)
#define OPBIT1 (1 << 7)

void init_io(void)
{
	// 1 = output, 0 = input
	DDRC |= OPBIT0;
	DDRB |= OPBIT1;
	DDRA |= 0b00111111;
}

void show_recv_output(void)
{
	if (IPBIT0)
	{
		PORTC |= OPBIT0;
	}
	else
	{
		PORTC &= ~OPBIT0;
	}
	if (IPBIT1)
	{
		PORTB |= OPBIT1;
	}
	else
	{
		PORTB &= ~OPBIT1;
	}
}
void generate_square_waves(void)
{
	static int hus_cnt /* hundred us */ = 0;
#ifndef ONE_KHZ
	static int period[] = {10, 20, 40, 50, 60, 80}; /* In 100us units - for 100Hz */
#else
	static int period[] = {1, 2, 4, 5, 6, 8}; /* In 100us units - for 1KHz */ // Will be not accurate
#endif

	int i;

	for (i = 0; i < 6; i++)
	{
		if (hus_cnt < period[i])
			PORTA |= (1 << i);
		else
			PORTA &= ~(1 << i);
	}
	hus_cnt++;
#ifndef ONE_KHZ
	if (hus_cnt == 100) // 100us from delay * 100 cnt = 10ms period === 100Hz cycle/frequency
#else
	if (hus_cnt == 10) // 100us from delay * 10 cnt = 1ms period === 1KHz cycle/frequency
						// Will be not accurate, as comparable to the time taken by instructions above
#endif
		hus_cnt = 0;
}

int main(void)
{
	init_io();

	while (1)
	{
		show_recv_output();
		generate_square_waves();
		_delay_us(100);
	}

	return 0;
}
