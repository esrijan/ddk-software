/*
 * Copyright (C) eSrijan Innovations Private Limited
 * 
 * Author: Anil Kumar Pugalia <anil_pugalia@eSrijan.com>
 *
 * Licensed under: JSL (See LICENSE file for details)
 *
 * ATmega16/32
 * 
 * Generates various waveforms @ 1KHz:
 * + Various duty cycle square waves on PA0-3
 * + Sine wave on OC2/PD7, using PWM on Timer 2
 *
 * The timing is achieved using timer interrupt handler triggering at a 10KHz frequency, i.e. every 100us.
 */

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#define ARR_SIZE(a) (sizeof(a) / sizeof(*a))

#if (F_CPU == 16000000)
#define PRESCALAR_BITS 0b011
#define PRESCALAR 64 // Gives 250KHz clock
#else
#error Non-supported clock frequency
#endif

#define ONE_KHZ
#define DC_OFF 127 /* Centered */
#define AMPLIFICATION 5 /* Should range from 1 to 11, for 8-bit values between 0 & 2*(11*10) */

void generate_square_waves(int hus_cnt);
void generate_amp_based_symmetric_wave(int hus_cnt, int amplitude[], int period);
void generate_amp_based_wave(int hus_cnt, int amplitude[], int period);

/* For amp_based_symmetric waves. Period = 4 * (l - 1) * 100us */
int sine[] = {0, 3, 6, 8, 9, 10}; // 0, 18, 36, 54, 72, 90 degrees
//int sine[] = {0, 31, 59, 81, 95, 100}; // 0, 18, 36, 54, 72, 90 degrees
//int sine[] = {0, 16, 31, 45, 59, 71, 81, 89, 95, 99, 100}; // 0, 9, 18, ..., 90 degrees
int tri[] = {0, 2, 4, 6, 8, 10};
int sqr[] = {10, 10, 10, 10, 10, 10};
/* For amp_based_symmetric waves. Period = l * 100us */
int tri_c[] = {0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7, 8, 8, 9, 9, 10, 10, 8, 6, 4, 2};
//int tri_c[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 8, 6, 4, 2};
//int ramp_c[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 0, 0, 0, 0};
int ramp_c[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
int sqr_c[] = {10, 10, 10, 10, 0, 0, 0, 0, 0, 0};
//int sqr_c[] = {10, 10, 10, 10, 10, 0, 0, 0, 0, 0};

ISR(TIMER0_COMP_vect)
{
	static int hus_cnt /* hundred us */ = 0;

	generate_square_waves(hus_cnt);
	generate_amp_based_symmetric_wave(hus_cnt, sine, ARR_SIZE(sine));
	//generate_amp_based_symmetric_wave(hus_cnt, tri, ARR_SIZE(tri));
	//generate_amp_based_symmetric_wave(hus_cnt, sqr, ARR_SIZE(sqr));
	//generate_amp_based_wave(hus_cnt, tri_c, ARR_SIZE(tri_c));
	//generate_amp_based_wave(hus_cnt, ramp_c, ARR_SIZE(ramp_c));
	//generate_amp_based_wave(hus_cnt, sqr_c, ARR_SIZE(sqr_c));

	hus_cnt++;
#ifndef ONE_KHZ
	if (hus_cnt == 100) // 100us from delay * 100 cnt = 10ms period === 100Hz cycle/frequency
#else
	if (hus_cnt == 10) // 100us from delay * 10 cnt = 1ms period === 1KHz cycle/frequency
#endif
		hus_cnt = 0;
}

void set_amplitude_centred(uint8_t a)
{
	OCR2 = (DC_OFF + a); // DC shift by peak to get it centered around DC_OFF
}
void set_amplitude(uint8_t a)
{
	OCR2 = 2 * a; // Double the room with same values as for symmetric waves, so let's amplify twice
}
void init_pwm(void)
{
	DDRD |= 0b10000000; // PD7

	set_amplitude(0);

	// Setup OC2 on PD7
	TCCR2 = (1 << WGM21) | (1 << WGM20); /* Fast PWM */
	TCCR2 |= (2 << COM20); /* Clear on Match */
	TCCR2 |= (1 << CS20); /* No prescaling => Clock @ F_CPU */ // Starts the PWM
}
void init_timer(void) /* Setting Timer 0 for trigger every 1ms */
{
	sei(); // Enable global interrupts
	TIMSK |= (1 << OCIE0); // Enable Compare Match interrupt
	/*
	 * Pre-scaled clock = F_CPU / PRESCALAR
	 * => Each timer counter increment takes PRESCALAR / F_CPU seconds
	 * => Formula for timer expiry interval is OCR0 (top timer count) * PRESCALAR / F_CPU
	 * Example: For F_CPU = 16MHz, PRESCALAR = 64
	 * Pre-scaled clock = 16MHz / 64 = 250KHz
	 * => Each timer counter increment takes 1/250KHz = 4us
	 * => Formula for timer expiry interval is OCR0 (top timer count) * 4us
	 * Example: For 100us = OCR0 * 4us, i.e. OCR0 = 25
	 */
	OCR0 = (F_CPU / PRESCALAR) / 10000; /* 1/10000 for 100us */
	/*
	 * Setting & Starting the Timer/Counter0 in CTC (Clear Timer on Compare) (non-PWM) for
	 * controlling timer expiry interval, directly by the compare register &
	 */
	TCCR0 = (1 << WGM01) | (0 << WGM00) | PRESCALAR_BITS;
}
void init_device(void)
{
	// 1 = output, 0 = input
	DDRA |= 0b11111111;

	init_pwm();
	init_timer();
}

void generate_square_waves(int hus_cnt)
{
#ifndef ONE_KHZ
	static int period[] = {10, 20, 50, 80}; /* In 100us units - for 100Hz */
#else
	static int period[] = {1, 2, 5, 8}; /* In 100us units - for 1KHz */ // Will be not accurate
#endif

	int i;

	for (i = 0; i < 4; i++)
	{
		if (hus_cnt < period[i])
			PORTA |= (1 << i);
		else
			PORTA &= ~(1 << i);
	}
}
void generate_amp_based_symmetric_wave(int hus_cnt, int amplitude[], int period)
{
	static int cycle = 0;
	static int i = 0;
	int amp;

	// Currently, let i change every hus_cnt, i.e. every 100us.
	(void)(hus_cnt);
	switch (cycle)
	{
		case 0:
			amp = amplitude[i];
			if (i == (period - 1))
			{
				cycle = 1;
				i--;
			}
			else
				i++;
			break;
		case 1:
			amp = amplitude[i];
			if (i == 0)
			{
				cycle = 2;
				i++;
			}
			else
				i--;
			break;
		case 2:
			amp = -amplitude[i];
			if (i == (period - 1))
			{
				cycle = 3;
				i--;
			}
			else
				i++;
			break;
		case 3:
			amp = -amplitude[i];
			if (i == 0)
			{
				cycle = 0;
				i++;
			}
			else
				i--;
			break;
		default: // Should not enter
			amp = 0;
			break;
	}
	set_amplitude_centred(AMPLIFICATION * amp);
}
void generate_amp_based_wave(int hus_cnt, int amplitude[], int period)
{
	static int i = 0;

	// Currently, let i change every hus_cnt, i.e. every 100us.
	(void)(hus_cnt);
	set_amplitude(AMPLIFICATION * amplitude[i]);
	if (++i == period)
	{
		i = 0;
	}
}

int main(void)
{
	init_device();

	while (1)
	{
		_delay_ms(500);
	}

	return 0;
}
