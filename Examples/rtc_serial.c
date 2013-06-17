/*
 * Copyright (C) eSrijan Innovations Private Limited
 * 
 * Author: Anil Kumar Pugalia <anil_pugalia@eSrijan.com>
 *
 * Licensed under: JSL (See LICENSE file for details)
 *
 * ATmega16/32
 * 
 * AVR initializes the RTC DS1307 to fixed date & time and then reads it every
 * once in a while and then outputs on the serial port.
 *
 * RTC DS1307 assumed to be connected as an I2C slave with AVR as I2C master.
 *
 * RTC chip has also been set to generate a 1Hz square wave on its SQW/OUT pin.
 *
 * For trying this example on DDK's, make sure of the following:
 * 	DDK v1.1: RX jumper should be on the left-most pin pair.
 * 	DDK v2.1: Both RX-TX jumpers should be on the right-most pin pairs.
 */

#include <avr/io.h>
#include <util/delay.h>

#include "rtc.h"
#include "serial.h"

char date[] = "01.06.2075 Sat";
char time[] = "03:07:00";

int main(void)
{
	uint8_t is_halted;

	rtc_init();
	usart_init(9600);

	/* Initialize the clock to 2075.June.1st Sat 03:07:00 */
	if (rtc_set(75, 6, 1, 7, 3, 7, 0) == 0)
	{
		usart_tx(date);
		usart_byte_tx(' ');
		usart_tx(time);
	}
	else
	{
		usart_tx("RTC set failed. Halting ... ");
		while (1);
	}
	usart_byte_tx('\n');
	rtc_set_sq_wave(hz_1);

	while (1)
	{
		if (rtc_get_str(&is_halted, date, time) == 0)
		{
			usart_tx(date);
			usart_byte_tx(' ');
			usart_tx(time);
		}
		else
			usart_tx("RTC read failed");
		usart_byte_tx('\n');
		_delay_ms(5000);
	}

	return 0;
}
