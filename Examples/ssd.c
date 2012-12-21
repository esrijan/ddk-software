/*
 * Copyright (C) eSrijan Innovations Private Limited
 * 
 * Author: Anil Kumar Pugalia <anil_pugalia@eSrijan.com>
 *
 * Licensed under: JSL (See LICENSE file for details)
 *
 * ATmega48/88/168
 * 
 * SSD Display with map.
 * PD0-PD7 are assumed to be connected to SSD's A-H, and each of the SSD's
 * common anodes to Vcc through 150 ohms resistor. Maximum current flowing
 * through each of the 2 resistors in this configuration is ~20mA.
 */

#include <avr/io.h>
#include <util/delay.h>

void init_display(void)
{
	DDRD = 0b11111111; // All outputs
}

uint8_t ssd_map(char c)
{
	switch (c)
	{
		case '0':
			return 0x3F;
		case '1':
			return 0x06;
		case '2':
			return 0x5B;
		case '3':
			return 0x4F;
		case '4':
			return 0x66;
		case '5':
			return 0x6D;
		case '6':
			return 0x7D;
		case '7':
			return 0x07;
		case '8':
			return 0x7F;
		case '9':
			return 0x6F;
		case '.':
			return 0x80;
		case '-':
			return 0x40;
		case ',':
			return 0x88;
		default:
			return 0x12;
	}
}

void display_ssd(uint8_t digit)
{
	PORTD = ~digit;
}

int main(void)
{
	char i;

	init_display();

	while (1)
	{
		for (i = 0; i < 8; i++)
		{
			display_ssd(1 << i);
			_delay_ms(500);
		}
		for (i = '0'; i <= '9'; i++)
		{
			display_ssd(ssd_map(i));
			_delay_ms(500);
		}
		display_ssd(ssd_map('.'));
		_delay_ms(500);
		display_ssd(ssd_map('-'));
		_delay_ms(500);
		display_ssd(ssd_map(','));
		_delay_ms(500);
		display_ssd(ssd_map(0));
		_delay_ms(500);
	}

	return 0;
}
