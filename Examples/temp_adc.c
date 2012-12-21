/*
 * Copyright (C) eSrijan Innovations Private Limited
 * 
 * Author: Anil Kumar Pugalia <anil_pugalia@eSrijan.com>
 *
 * Licensed under: JSL (See LICENSE file for details)
 *
 * ATmega48/88/168, ATmega16/32
 * 
 * Sense & Measure temperature using an LM35, and display on the LCD
 * LM35 analog output is assumed to be connected to ADC0 (PA0) for analog to
 * digital conversion. 0V = 2C; and then every 10mV = 1C
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
	//DIDR0 = (1 << ADC0D); // Disable digital i/p on PA0 for lower power consumption
}

int main(void)
{
	uint16_t reading, milli_voltage;
	char buf[16];

	init_adc();
	clcd_init();

	clcd_move_to(0);
	clcd_print_string("Voltage: ");
	clcd_move_to(16);
	clcd_print_string("Temp.  : ");
	while (1)
	{
		while (!(ADCSRA & (1 << ADIF)))
			;
		ADCSRA |= (1 << ADIF);
		reading = ADCL;
		reading += ADCH << 8; // Multiply ADCH by 256
		milli_voltage = 5000UL * reading / 1023;
		sprintf(buf, "%4d mV", milli_voltage);
		clcd_move_to(9);
		clcd_print_string(buf);
		sprintf(buf, "%4d C", 2 + milli_voltage / 10); // As per datasheet
		clcd_move_to(25);
		clcd_print_string(buf);
		_delay_ms(1000);
		// Restart ADC conversion
		ADCSRA |= (1 << ADSC);
	}

	return 0;
}
