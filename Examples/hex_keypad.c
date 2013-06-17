/*
 * Copyright (C) Vinay V S, 2011. Email: vinayvs1991@gmail.com
 *
 * Updated by: Anil Kumar Pugalia <anil_pugalia@eSrijan.com>
 *
 * Licensed under: JSL (See LICENSE file for details)
 *
 * ATmega16/32
 *
 * Hex Keypad interface with CLCD & EEPROM. CLCD assumed to be connected as
 * specified in the comments of clcd.c.
 *
 * Keypad has 16 keys: 0, 1, 2, 3, ...... , A, B, C, D, E, F.
 */

#include <avr/io.h>
#include <util/delay.h>
#include <avr/eeprom.h>

#include "clcd.h"

unsigned char keypressed[] = {'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'};

void init_io()
{
	// 1 = output, 0 = input
	DDRA |= 0xF0;  // DDRA |= 0b11110000
}

unsigned char keypad(void)
{
	unsigned char value,i;

	while (1)
	{
		for (i = 0; i < 4; i++)
		{
			PORTA = (0x80 >> i);
			_delay_ms(20); // key debounce time > 10ms
			value = PINA & 0x0F;

			switch(value)
			{
				case 0x08:
					clcd_data_wr(keypressed[i+0]);
					return keypressed[i+0];
					break;
				case 0x04:
					clcd_data_wr(keypressed[i+4]);
					return keypressed[i+4];
					break;
				case 0x02:
					clcd_data_wr(keypressed[i+8]);
					return keypressed[i+8];
					break;
				case 0x01:
					clcd_data_wr(keypressed[i+12]);	
					return keypressed[i+12];
					break;

				default:
					//clcd_data_wr('x');	
					break;
			}
		}
	}
}

int main(void)
{ 
	int i;
	unsigned char data[16];

	init_io();
	clcd_init();
	clcd_cls();

	for (i = 0; i < 16; i++)
	{
		data[i] = eeprom_read_byte((uint8_t *)i);
		clcd_data_wr(data[i]);	
	}

	i = 0;
	while (1)
	{
		data[i] = keypad();
		eeprom_write_byte((uint8_t *)i, data[i]);
		if (++i == 16)
		{
			i = 0;
		}
	}
	return 0;
}
