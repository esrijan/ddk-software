/*
 * Copyright (C) eSrijan Innovations Private Limited
 * 
 * Author: Anil Kumar Pugalia <anil_pugalia@eSrijan.com>
 *
 * Licensed under: JSL (See LICENSE file for details)
 *
 * ATmega48/88/168, ATmega16/32
 * 
 * AVR initializes the RTC DS1307 to fixed date & time and then reads it every
 * 1/2 sec and then outputs on the CLCD.
 *
 * RTC DS1307 assumed to be connected as an I2C slave with AVR as I2C master.
 * CLCD assumed to be connected as specified in the comments of clcd.c.
 *
 * RTC chip has also been set to generate a 1Hz square wave on its SQW/OUT pin.
 */

#include <avr/io.h>
#include <util/delay.h>

#include "rtc.h"
#include "clcd.h"

char date[] = "01.06.2075 Sat";
char time[] = "03:07:00";

int main(void)
{
	uint8_t is_halted;

	rtc_init();
	clcd_init();

	/* Initialize the clock to 2075.June.1st Sat 03:07:00 */
	if (rtc_set(75, 6, 1, 7, 3, 7, 0) == 0)
	{
		clcd_move_to(0);
		clcd_print_string(date);
		clcd_move_to(16);
		clcd_print_string(time);
	}
	else
	{
		clcd_move_to(25);
		clcd_print_string("init :(");
		while (1);
	}
	rtc_set_sq_wave(hz_1);

	while (1)
	{
		if (rtc_get_str(&is_halted, date, time) == 0)
		{
			clcd_move_to(0);
			clcd_print_string(date);
			clcd_move_to(16);
			clcd_print_string(time);
			if (is_halted)
			{
				clcd_move_to(25);
				clcd_print_string("Halted");
			}
		}
		else
		{
			clcd_move_to(25);
			clcd_print_string("read :(");
		}

		_delay_ms(200);
	}

	return 0;
}
