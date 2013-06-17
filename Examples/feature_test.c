/*
 * Copyright (C) eSrijan Innovations Private Limited
 * 
 * Author: Anil Kumar Pugalia <anil_pugalia@eSrijan.com>
 *
 * Licensed under: JSL (See LICENSE file for details)
 *
 * ATmega16/32
 * 
 * Basic feature tester app over serial for DDK v2.1
 * Both RX-TX jumpers should be on the right-most pin pairs.
 * CLCD may be connected as specified in the comments of clcd.c.
 */

#include <avr/io.h>
#include <stdio.h>

#include "ddk.h"
#include "serial.h"
#include "rtc.h"
#include "clcd.h"

static uint8_t is_halted, d, mo, y, dy, h, m, s;
static char *day[] = { "", "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" }; 
static int is_lcd_enabled = 0;

static void init_io(void)
{
	rtc_init();
	rtc_set_sq_wave(hz_1);

	// 1 = output, 0 = input
	BUTTON_INIT;
	PORTB |= (0b00010000); // Pull-up on PB4
	DDRB |= 0b00000000;
}

void display_date_n_time(void)
{
	char date[] = "dd.mm.yyyy DDD", time[] = "hh:mm:ss";

	sprintf(date, "%02d.%02d.20%02d %3s", d, mo, y, day[dy]);
	if (is_lcd_enabled)
	{
		clcd_move_to(0);
		clcd_print_string(date);
	}
	else
	{
		usart_tx("RTC: ");
		usart_tx(date);
	}
	sprintf(time, "%02d:%02d:%02d", h, m, s);
	if (is_lcd_enabled)
	{
		clcd_move_to(16);
		clcd_print_string(time);
	}
	else
	{
		usart_tx(" ");
		usart_tx(time);
		usart_tx(".\n");
	}
}

int main(void)
{
	int i;
	char str[16];

	usart_init(9600);
	init_io();

	usart_byte_rx(); // Waiting for a character, typically an <Enter>
	usart_tx("Welcome to eSrijan's DDK v2.1 tester\n");

	while (1)
	{
		if (BUTTON_PRESSED)
		{
			usart_tx("Button pressed\n");
		}
		usart_tx("1: Read RTC\n");
		usart_tx("2: Set RTC\n");
		usart_tx("3: IR On\n");
		usart_tx("4: IR Off\n");
		usart_tx("5: Enable CLCD\n");
		usart_tx("6: Disable CLCD\n");
		usart_tx("eSrijan> ");
		i = -1;
		do
		{
			if (i < 16) i++;
			if (i < 15) str[i] = usart_byte_rx();
		} while (str[i] != '\n');
		str[i] = 0;
		if (i != 1)
		{
			//usart_byte_tx('0' + i);
			continue;
		}
		switch (str[0])
		{
			case '1':
				if (rtc_get(&is_halted, &y, &mo, &d, &dy, &h, &m, &s) == 0)
				{
					if (is_halted)
					{
						usart_tx("Halted ");
					}
					display_date_n_time();
				}
				else
				{
						usart_tx("RTC read error.\n");
				}
				break;
			case '2':
				usart_tx("Enter RTC value in DD.MM.YY d hh:mm:ss format: ");
				d = 10 * (usart_byte_rx() - '0');
				d += (usart_byte_rx() - '0');
				usart_byte_rx(); // Ignore '.'
				mo = 10 * (usart_byte_rx() - '0');
				mo += (usart_byte_rx() - '0');
				usart_byte_rx(); // Ignore '.'
				y = 10 * (usart_byte_rx() - '0');
				y += (usart_byte_rx() - '0');
				usart_byte_rx(); // Ignore ' '
				dy = usart_byte_rx() - '0';
				usart_byte_rx(); // Ignore ' '
				h = 10 * (usart_byte_rx() - '0');
				h += (usart_byte_rx() - '0');
				usart_byte_rx(); // Ignore ':'
				m = 10 * (usart_byte_rx() - '0');
				m += (usart_byte_rx() - '0');
				usart_byte_rx(); // Ignore ':'
				s = 10 * (usart_byte_rx() - '0');
				s += (usart_byte_rx() - '0');
				usart_byte_rx(); // Ignore '\n'
				if (rtc_set(y, mo, d, dy, h, m, s) == 0)
				{
					usart_tx("RTC time set.\n");
				}
				else
				{
						usart_tx("RTC read error.\n");
				}
				break;
			case '3':
				DDRB |= _BV(PB3);
				PORTB |= _BV(PB3);
				usart_tx("IR is now on.\n");
				break;
			case '4':
				PORTB &= ~_BV(PB3);
				DDRB &= ~_BV(PB3);
				usart_tx("IR is now off.\n");
				break;
			case '5':
				clcd_init();
				is_lcd_enabled = 1;
				usart_tx("CLCD is now enabled.\n");
				break;
			case '6':
				clcd_cls();
				is_lcd_enabled = 0;
				usart_tx("CLCD is now disabled.\n");
				break;
			default:
				break;
		}
	}

	return 0;
}
