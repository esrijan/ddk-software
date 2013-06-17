/*
 * Copyright (C) eSrijan Innovations Private Limited
 * 
 * Author: Anil Kumar Pugalia <anil_pugalia@eSrijan.com>
 *
 * Licensed under: JSL (See LICENSE file for details)
 *
 * ATmega16/32
 * 
 * Controls 2 DC motors fitted onto the two wheels of a robot using an L293D
 * chip. Uses LED (on PB7) for debugging.
 */

#include <avr/io.h>
#include <util/delay.h>

#include "ddk.h"

typedef enum
{
	motor_1,
	motor_2
} Motor;
typedef enum
{
	rev = -1,
	stop,
	fwd
} Dir;

void init_io(void)
{
	// 1 = output, 0 = input
	BUTTON_INIT;
	DDRB |= 0b10000000;
	DDRA |= 0b00111111;
}

void ctl_motor(int motor, int dir)
/* M-: 0 1 1; M0: 0 0 0; M+: 1 1 0 */
{
	uint8_t ctl = 0;

	switch (dir)
	{
		case rev:
			ctl = 0b011;
			break;
		case stop:
			ctl = 0b000;
			break;
		case fwd:
			ctl = 0b110;
			break;
	}
	if (motor == motor_1)
	{
		PORTA = (PORTA & ~0b111) | ctl;
	}
	else
	{
		PORTA = (PORTA & ~(0b111 << 3)) | (ctl << 3);
	}
}

int main(void)
{
	init_io();

	_delay_ms(5000);

	ctl_motor(motor_1, fwd);
	ctl_motor(motor_2, rev);

	while (1)
	{
		if (BUTTON_PRESSED)
		{
			PORTB |= (1 << 7);
			ctl_motor(motor_1, stop);
			ctl_motor(motor_2, stop);
		}
		else
		{
			PORTB &= ~(1 << 7);
		}
		_delay_ms(20);
	}

	return 0;
}
