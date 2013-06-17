/*
 * Copyright (C) eSrijan Innovations Private Limited
 * 
 * Author: Anil Kumar Pugalia <anil_pugalia@eSrijan.com>
 *
 * Licensed under: JSL (See LICENSE file for details)
 *
 * ATmega48/88/168, ATmega16/32
 *
 * Character LCD Demo. CLCD assumed to be connected as specified in the comments
 * of clcd.c. A switch connected on PB2 can provide extra demo.
 */

#include <avr/io.h>
#include <util/delay.h>
#include <stdlib.h>

#include "ddk.h"
#include "clcd.h"

#define NUM_MSGS 6

static char msgs[NUM_MSGS][15] = {"apple", "banana", "pineapple", "mango", "watermelon", "pear"};

int main(void)
{
	uint8_t i, pos;

	clcd_init();
	/*
	 * Optionally, now set up our application-specific display settings,
	 * overriding whatever we did in clcd_init()
	 */
	//clcd_cmd_wr(0x0F); // All On: Cursor, Display, Blink. (nasty!)

	BUTTON_INIT;
	if (BUTTON_PRESSED)
	{
		while (1)
		{
			/* Print a random message from the array */
			clcd_print_string(msgs[random() % NUM_MSGS]);
			_delay_ms(1000);

			/* Print some dots individually */
			for (i = 0; i < 3; i++)
			{
				clcd_data_wr('.');
				_delay_ms(100);
			}

			/* Print something on the display's second line */
			clcd_move_to(16);
			clcd_print_string("Score: 6/7");
			_delay_ms(1000);

			/* Scroll entire display 20 chars to left, delaying 50ms each inc */
			clcd_scroll_left(20, 50);
			clcd_cls();
		}
	}
	else
	{
		clcd_move_to(16);
		clcd_print_string("LDD Kit v1.1");
		_delay_ms(1000);
		i = 0;
		pos = 0;
		clcd_move_to(0);
		while (1)
		{
			clcd_data_wr(i);
			_delay_ms(100);
			i++;
#ifndef CLCD_AUTO_POS
			if (++pos == 16)
			{
				pos = 0;
				clcd_move_to(pos);
			}
#endif
		}
	}

	return 0;
}
