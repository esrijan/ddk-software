/*
 * Copyright (C) eSrijan Innovations Private Limited
 * 
 * Author: Anil Kumar Pugalia <anil_pugalia@eSrijan.com>
 *
 * Licensed under: JSL (See LICENSE file for details)
 *
 * ATmega48/88/168, ATmega16/32
 * 
 * Detect light intensity using an LDR, and accordingly glow an LED.
 * CLCD is used for extra info display.
 * Connection Vcc -- 470 ohms (R) -- LDR -- Gnd.
 * Using ADC0 for analog voltage input at the junction of R & LDR.
 * LED is connected to PB7.
 * CLCD assumed to be connected as specified in the comments of clcd.c.
 */

#include <avr/io.h>
#include <stdio.h>
#include <util/delay.h>

#include "clcd.h"

void init_adc(void)
{
	// ADC0 to be read for input between GND-AVCC
	ADMUX = (1 << REFS0) | (0 << MUX0); // Select ADC0
	// ADC clock should be between 50KHz to 200KHz
#if (F_CPU < 100000)
#error Processor frequency should be between 100 KHz & 20 MHz
#elif (F_CPU <= 400000)
	ADCSRA = (1 << ADPS0); // Prescaler is /2
#elif (F_CPU <= 800000)
	ADCSRA = (2 << ADPS0); // Prescaler is /4
#elif (F_CPU <= 1600000)
	ADCSRA = (3 << ADPS0); // Prescaler is /8
#elif (F_CPU <= 3200000)
	ADCSRA = (4 << ADPS0); // Prescaler is /16
#elif (F_CPU <= 6400000)
	ADCSRA = (5 << ADPS0); // Prescaler is /32
#elif (F_CPU <= 12800000)
	ADCSRA = (6 << ADPS0); // Prescaler is /64
#elif (F_CPU <= 20000000)
	ADCSRA = (7 << ADPS0); // Prescaler is /128
#else
#error Processor frequency should be between 100 KHz & 20 MHz
#endif
	//ADCSRB = (0 < ADTS0); // Free running mode
	// Enable & Start Conversion on the ADC // with Auto Trigger
	ADCSRA |= (1 << ADEN) | (1 << ADSC); // | (1 << ADATE);
	//DIDR0 = (1 << ADC0D); // Disable digital i/p on ADC0 for lower power consumption

	// Enable LED pin as output on PB7 & switch it off by default
	PORTB &= ~(1 << 7); // LED Off
	DDRB |= (1 << 7);
}

int main(void)
{
	uint16_t luminance;
	char buf[16];

	init_adc();
	clcd_init();

	clcd_move_to(0);
	clcd_print_string("Luminance: ");
	clcd_move_to(16);
	clcd_print_string("Lights   : ");
	while (1)
	{
		while (!(ADCSRA & (1 << ADIF)))
			;
		ADCSRA |= (1 << ADIF);
		// Multiply ADCL by 100 & Divide by 1024 (actually 1023)
		luminance = (ADCL * 100) >> 10;
		// Multiply ADCH by 256, then 100 & Divide by 1024 (actually 1023)
		luminance += (ADCH * 100) >> 2;
		luminance = 100 - luminance;
		sprintf(buf, "%d%%", luminance);
		clcd_move_to(11);
		clcd_print_string(buf);
		clcd_move_to(27);
		if (luminance < 50) // Darkness
		{
			PORTB |= (1 << 7); // LED On
			clcd_print_string(" On");
		}
		else
		{
			PORTB &= ~(1 << 7); // LED Off
			clcd_print_string("Off");
		}
		_delay_ms(1000);
		// Restart ADC conversion
		ADCSRA |= (1 << ADSC);
	}

	return 0;
}
