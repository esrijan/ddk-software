/*
 * Copyright (C) eSrijan Innovations Private Limited
 * 
 * Author: Anil Kumar Pugalia <anil_pugalia@eSrijan.com>
 *
 * Licensed under: JSL (See LICENSE file for details)
 *
 * ATmega16/32
 * 
 * Sets up DDK v2.1 as an LCD Clock (based on RTC DS1307)
 * Mode change using DOWNLOAD (PB4). Value change using BUTTON (PB2).
 *
 * CLCD assumed to be connected as specified in the comments of clcd.c.
 *
 * Additionally, sets the RTC DS1307 to generate a 1Hz square wave on its
 * SQW/OUT pin.
 */

#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>

#include "ddk.h"
#include "rtc.h"
#include "clcd.h"

static uint8_t is_halted, d, mo, y, dy, h, m, s;
static char *day[] = { "", "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" }; 
static enum mode
{
	display,
	setting,
	total_modes
} cur_mode;
static enum
{
	set_date,
	set_month,
	set_year,
	set_day,
	set_hour,
	set_min,
	set_sec,
	total_set_modes
} cur_set_mode;

static void init_io(void)
{
	rtc_init();
	rtc_set_sq_wave(hz_1);
	clcd_init();

	// 1 = output, 0 = input
	BUTTON_INIT;
	PORTB |= (0b00010000); // Pull-up on PB4
	DDRB |= 0b00000000;
}

void display_date_n_time(void)
{
	char date[] = "dd.mm.yyyy DDD", time[] = "hh:mm:ss";

	sprintf(date, "%02d.%02d.20%02d %3s", d, mo, y, day[dy]);
	clcd_move_to(0);
	clcd_print_string(date);
	sprintf(time, "%02d:%02d:%02d", h, m, s);
	clcd_move_to(16);
	clcd_print_string(time);
}

void set_clock_mode(enum mode m)
{
	switch (m)
	{
		case display:
			cur_set_mode = set_date;
			cur_mode = m;
			clcd_cmd_wr(0x0C); /* 00001 - DisplayOn - CursorOff - CursorBlinkOff */
			clcd_cls();
			break;
		case setting:
			cur_mode = m;
			clcd_cmd_wr(0x0F); /* 00001 - DisplayOn - CursorOn - CursorBlinkOn */
			/* Scroll entire display 16 chars to left, delaying 20ms each inc */
			//clcd_scroll_left(16, 20);
			clcd_cls();
			break;
		default:
			break;
	}
}

int main(void)
{
	init_io();

	if (rtc_get(&is_halted, &y, &mo, &d, &dy, &h, &m, &s) != 0)
	{
		clcd_move_to(25);
		clcd_print_string("Clk :(");
		while (1);
	}
	if (is_halted) /* Clock is halted. So, set & enable it first */
	{
		set_clock_mode(setting);
	}
	else
	{
		set_clock_mode(display);
	}

	while (1)
	{
		switch (cur_mode)
		{
			case display:
				if (rtc_get(&is_halted, &y, &mo, &d, &dy, &h, &m, &s) == 0)
				{
					if (is_halted)
					{
						set_clock_mode(setting);
					}
					else
					{
						display_date_n_time();
						_delay_ms(200);
						break;
					}
				}
				else
				{
					clcd_move_to(25);
					clcd_print_string("Read :(");
					_delay_ms(1000);
					break;
				}
				/* Fall through to setting, as it is halted */
			case setting:
				display_date_n_time();
				switch (cur_set_mode)
				{
					case set_date:
						clcd_move_to(1); // Positioning the cursor
						_delay_ms(100);
						if (BUTTON_PRESSED) // BUTTON pressed
							if (d++ == 31) d = 1;
						break;
					case set_month:
						clcd_move_to(4); // Positioning the cursor
						_delay_ms(100);
						if (BUTTON_PRESSED) // BUTTON pressed
							if (mo++ == 12) mo = 1;
						break;
					case set_year:
						clcd_move_to(9); // Positioning the cursor
						_delay_ms(100);
						if (BUTTON_PRESSED) // BUTTON pressed
							if (y++ == 99) y = 0;
						break;
					case set_day:
						clcd_move_to(13); // Positioning the cursor
						_delay_ms(100);
						if (BUTTON_PRESSED) // BUTTON pressed
							if (dy++ == 7) dy = 1;
						break;
					case set_hour:
						clcd_move_to(17); // Positioning the cursor
						_delay_ms(100);
						if (BUTTON_PRESSED) // BUTTON pressed
							if (h++ == 23) h = 0;
						break;
					case set_min:
						clcd_move_to(20); // Positioning the cursor
						_delay_ms(100);
						if (BUTTON_PRESSED) // BUTTON pressed
							if (m++ == 59) m = 0;
						break;
					case set_sec:
						clcd_move_to(23); // Positioning the cursor
						_delay_ms(100);
						if (BUTTON_PRESSED) // BUTTON pressed
							if (s++ == 59) s = 0;
						break;
					default:
						clcd_move_to(25);
						clcd_print_string("Inv SM");
						_delay_ms(1000);
						break;
				}
				//clcd_cls();
				break;
			default:
				clcd_move_to(25);
				clcd_print_string("Inv M");
				_delay_ms(1000);
				break;
		}

		if (!(PINB & (1 << PB4))) // DOWNLOAD pressed
		{
			if (cur_mode == display)
			{
				set_clock_mode(setting);
			}
			else // (cur_mode == setting)
			{
				if (++cur_set_mode == total_set_modes)
				{
					if (rtc_set(y, mo, d, dy, h, m, s) != 0)
					{
						clcd_move_to(25);
						clcd_print_string("Set :(");
						_delay_ms(1000);
					}
					set_clock_mode(display);
				}
			}
		}
	}

	return 0;
}
