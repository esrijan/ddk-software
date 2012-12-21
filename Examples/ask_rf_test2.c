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
 * The timing is achieved using timer interrupt handler triggering at a 10KHz frequency, i.e. every 100us.
 */

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#if (F_CPU == 16000000)
#define PRESCALAR_BITS 0b011
#define PRESCALAR 64 // Gives 250KHz clock
#else
#error Non-supported clock frequency
#endif

#define ONE_KHZ

#define IPBIT0 (PINA & (1 << 6))
#define IPBIT1 (PINA & (1 << 7))
#define OPBIT0 (1 << 0)
#define OPBIT1 (1 << 7)

void show_recv_output(void);
void generate_square_waves(void);

ISR(TIMER0_COMP_vect)
{
	show_recv_output();
	generate_square_waves();
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
	 * Example: For 1ms = OCR0 * 4us, i.e. OCR0 = 250
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
	DDRC |= OPBIT0;
	DDRB |= OPBIT1;
	DDRA |= 0b00111111;

	init_timer();
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
#endif
		hus_cnt = 0;
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
