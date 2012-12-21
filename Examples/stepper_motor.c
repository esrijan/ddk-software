/*
 * Copyright (C) eSrijan Innovations Private Limited
 * 
 * Author: Anil Kumar Pugalia <anil_pugalia@eSrijan.com>
 *
 * Licensed under: JSL (See LICENSE file for details)
 *
 * ATmega48/88/168, ATmega16/32
 * 
 * NB Tested but need a higher current power supply to drive the motor, or get
 * a lower current drawing motor
 * Controls a unipolar stepper motor with 3.125/3.75 degrees per rotation,
 * using PA0-3 as inputs to Darlington Array, outputs of which are connected to
 * the stepper motor represented by the following wires:
 * Type: 16PU-M301-G1 (Astrosyn) consumes ~1.5A @ 5V
 * + 1 - Coil11 - Pink; 2 - Coil12 - Yellow; 3 - Coil21 - Blue; 4 - Coil22 - Orange
 * + 5 - Black & 6 - White - Together
 * Type: 25BY4809 consumes ~0.15A @ 5V
 * + 1 - Coil11 - Red; 2 - Coil12 - Yellow; 3 - Coil21 - Black; 4 - Coil22 - White
 * + 5 - Orange & 6 - Brown - Together
 * Other connections in the schematic diagram
 * PB7 is being a toggling output for LED
 */

#include <avr/io.h>
#include <util/delay.h>

#define WIRE(i, val) ((val) << (i - 1))

void init_io(void)
{
	DDRA = 0b00001111; // 4 LSBs as outputs
	DDRB = 0b10000000; // MSB as output
	_delay_ms(500); // TODO: Is it needed?
}

int main(void)
{
	int speed_by_4 = 4;
	init_io();

	while (1)
	{
		PORTB = (0 << 7);
		PORTA = WIRE(1, 1) | WIRE(2, 0) | WIRE(3, 1) | WIRE(4, 0);
		_delay_ms(speed_by_4);
		PORTA = WIRE(1, 0) | WIRE(2, 1) | WIRE(3, 1) | WIRE(4, 0);
		_delay_ms(speed_by_4);
		PORTA = WIRE(1, 0) | WIRE(2, 1) | WIRE(3, 0) | WIRE(4, 1);
		_delay_ms(speed_by_4);
		PORTA = WIRE(1, 1) | WIRE(2, 0) | WIRE(3, 0) | WIRE(4, 1);
		_delay_ms(speed_by_4);
		PORTB = (1 << 7);
	}

	return 0;
}
