/*
 * Copyright (C) eSrijan Innovations Private Limited
 * 
 * Author: Anil Kumar Pugalia <anil_pugalia@eSrijan.com>
 *
 * Licensed under: JSL (See LICENSE file for details)
 *
 * ATmega16/32
 *
 * Menu on Character LCD controlled w/ 2 switches BUTTON & DOWNLOAD on DDK v2.1
 * Mode change using DOWNLOAD. Value change using BUTTON.
 *
 * CLCD assumed to be connected as specified in the comments of clcd.c.
 */

#include <avr/io.h>
#include <util/delay.h>

#include "ddk.h"
#include "clcd.h"

#define CNT(a) (sizeof(a) / sizeof(*a))

static enum
{
	display,
	set_fruit,
	set_day,
	total_modes
} cur_mode = display;
static char fruit[][10] = {"apple", "banana", "pineapple", "mango", "papaya", "pear"};
static char day[][10] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
static int fruit_i = 0, day_i = 0;

static void init_io(void)
{
	// 1 = output, 0 = input
	BUTTON_INIT;
	PORTB |= (0b00010000); // Pull-up on PB4
	DDRB |= 0b00000000;

	clcd_init();
}

int main(void)
{
	int next;

	init_io();

	while (1)
	{
		switch (cur_mode)
		{
			case display:
				/* Display the current values */
				clcd_move_to(0);
				clcd_print_string("Fruit: ");
				clcd_print_string(fruit[fruit_i]);
				clcd_move_to(16);
				clcd_print_string("Day: ");
				clcd_print_string(day[day_i]);
				_delay_ms(1000);
				break;
			case set_fruit:
				/* Display the fruit setting screen */
				clcd_move_to(0);
				clcd_print_string("Fruit: ");
				clcd_print_string(fruit[fruit_i]);
				clcd_move_to(16);
				clcd_print_string(" Next: ");
				next = ((fruit_i + 1) == CNT(fruit)) ? 0 : (fruit_i + 1);
				clcd_print_string(fruit[next]);
				clcd_move_to(15); // Positioning the cursor
				_delay_ms(100);
				if (BUTTON_PRESSED) // BUTTON pressed
				{
					if (++fruit_i == CNT(fruit))
						fruit_i = 0;
					clcd_cls();
				}
				break;
			case set_day:
				/* Display the day setting screen */
				clcd_move_to(0);
				clcd_print_string("Day: ");
				clcd_print_string(day[day_i]);
				clcd_move_to(16);
				clcd_print_string(" Next: ");
				next = ((day_i + 1) == CNT(day)) ? 0 : (day_i + 1);
				clcd_print_string(day[next]);
				clcd_move_to(15); // Positioning the cursor
				_delay_ms(100);
				if (BUTTON_PRESSED) // BUTTON pressed
				{
					if (++day_i == CNT(day))
						day_i = 0;
					clcd_cls();
				}
				break;
			default:
				clcd_move_to(0);
				clcd_print_string("Invalid Mode");
				_delay_ms(1000);
				break;
		}
		if (!(PINB & (1 << PB4))) // DOWNLOAD pressed
		{
			if (++cur_mode == total_modes)
			{
				cur_mode = display;
				clcd_cmd_wr(0x0C); /* 00001 - DisplayOn - CursorOff - CursorBlinkOff */
			}
			else
			{
				clcd_cmd_wr(0x0E); /* 00001 - DisplayOn - CursorOn - CursorBlinkOn */
			}
			/* Scroll entire display 16 chars to left, delaying 20ms each inc */
			clcd_scroll_left(16, 20);
			clcd_cls();
		}
	}

	return 0;
}
