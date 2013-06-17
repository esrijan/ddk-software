/*
 * Copyright (C) eSrijan Innovations Private Limited
 * 
 * Author: Anil Kumar Pugalia <anil_pugalia@eSrijan.com>
 *
 * Licensed under: JSL (See LICENSE file for details)
 *
 * ATmega48/88/168, ATmega16/32
 *
 * Character LCD Functions assuming the following:
 * + PORTD is LCD Data Bus. PD0-7 for 8-bit & PD4-7 for 4-bit. Default is 8-bit
 * 	> Define CLCD_FOUR_BIT for 4-bit
 * + PB0: Enable; PB1: Register Select; Read/Write is GND (as not using READ). Otherwise, PB2
 * NB Not using the LCD busy check by reading the LCD Data Bus. Rather, using
 * pre-determined delays, as the former was not properly working, possibly
 * because of switching between the input & output modes for the Port (D)
 * connected to LCD Data Bus, or may be the need for the un-mentioned init
 * sequence, which is done now. Moreover, it saves a pin
 */

#include <avr/io.h>
#ifndef NO_SYS_DELAY
#include <util/delay.h>
#else
#include "common.h"
#define _delay_ms delay_ms
#define _delay_us delay_us
#endif

#include "clcd.h"

// TODO: Ideally all delays should be in usecs only. So, could the _delay_ms(1) be replaced, accordingly
//#define CLCD_RW_CTL // TODO: Issues persisting. See comments above
#define CLCD_FOUR_BIT

#define LCDDREG DDRD
#define LCDDATA PORTD
#define LCDIBUS PIND

#define LCDCREG DDRB
#define LCDCTRL PORTB
#define CLCD_EN (1 << 0) /* PB0 */
#define CLCD_RS (1 << 1) /* PB1 */
#ifdef CLCD_RW_CTL
#define CLCD_RW (1 << 2) /* PB2 */
#endif
#define CLCD_CMD 0
#define CLCD_DATA 1

#ifdef CLCD_AUTO_POS
static uint8_t clcd_pos;
#endif

#ifdef CLCD_RW_CTL
static void clcd_wait_till_busy(void) // TODO: Not tested
// TODO: Ideally all delays in this function should be < 1us
{
	uint8_t done;

	done = 0;
	LCDCTRL &= ~CLCD_RS;
	LCDCTRL |= CLCD_RW;
#ifdef CLCD_FOUR_BIT
	LCDDREG &= ~0xF0;
#else
	LCDDREG = 0x00;
#endif
	while (!done)
	{
		LCDCTRL |= CLCD_EN;
		_delay_ms(1);
		if ((LCDIBUS & 0x80) == 0)
		{
			done = 1;
		}
		LCDCTRL &= ~CLCD_EN;
		_delay_ms(1);
#ifdef CLCD_FOUR_BIT
		LCDCTRL |= CLCD_EN;
		_delay_ms(1);
		if ((LCDIBUS & 0x00) == 0)
		{
			// Dummy Read
		}
		LCDCTRL &= ~CLCD_EN;
		_delay_ms(1);
#endif
	}
#ifdef CLCD_FOUR_BIT
	LCDDREG |= 0xF0;
#else
	LCDDREG = 0xFF;
#endif
}
#endif
static void clcd_wr(uint8_t type, uint8_t val)
{
#ifdef CLCD_RW_CTL
	clcd_wait_till_busy();
#endif
	if (type == CLCD_CMD)
	{
		LCDCTRL &= ~CLCD_RS;
	}
	else
	{
		LCDCTRL |= CLCD_RS;
	}
#ifdef CLCD_RW_CTL
	LCDCTRL &= ~CLCD_RW;
#endif
#ifdef CLCD_FOUR_BIT
	LCDDATA = (LCDDATA & ~0xF0) | (val & 0xF0);
#else
	LCDDATA = val;
#endif
	LCDCTRL |= CLCD_EN;
	_delay_us(1);
	LCDCTRL &= ~CLCD_EN;
	_delay_ms(1);
#ifdef CLCD_FOUR_BIT
#ifdef CLCD_RW_CTL
	clcd_wait_till_busy();
#endif
	if (type == CLCD_CMD)
	{
		LCDCTRL &= ~CLCD_RS;
	}
	else
	{
		LCDCTRL |= CLCD_RS;
	}
#ifdef CLCD_RW_CTL
	LCDCTRL &= ~CLCD_RW;
#endif
	LCDDATA = (LCDDATA & ~0xF0) | (val << 4);
	LCDCTRL |= CLCD_EN;
	_delay_us(1);
	LCDCTRL &= ~CLCD_EN;
	_delay_ms(1);
#endif
}

void clcd_cmd_wr(uint8_t cmd)
{
	clcd_wr(CLCD_CMD, cmd);
}
void clcd_home(void)
{
	clcd_cmd_wr(0x02);
#ifdef CLCD_AUTO_POS
	clcd_pos = 0;
#endif
}
#if 0
void clcd_move_one_to_right()
{
	clcd_cmd_wr(0x14);
}
#endif
void clcd_move_to(uint8_t position) /* 0-31 */
{
	register uint8_t ddram_addr;

	if (position > 31)
	{
		position = 0;
	}
#ifdef CLCD_AUTO_POS
	clcd_pos = position;
#endif
	if (position < 16)
	{
		ddram_addr = position;
	}
	else
	{
		ddram_addr = 0x40 + (position - 16);
	}
	ddram_addr |= 0x80 /* Position Cmd: 1 - DdramAddr */;
	clcd_cmd_wr(ddram_addr);
}
void clcd_scroll_left(int num_chars, int delay)
{
	int i;

	for (i = 0; i < num_chars; i++)
	{
		clcd_cmd_wr(0x18);
		_delay_ms(delay);
	}
}
void clcd_scroll_right(int num_chars, int delay)
{
	int i;

	for (i = 0; i < num_chars; i++)
	{
		clcd_cmd_wr(0x1C);
		_delay_ms(delay);
	}
}
void clcd_cls(void)
{
	clcd_cmd_wr(0x01);
#ifdef CLCD_AUTO_POS
	clcd_pos = 0;
#endif
	_delay_ms(1);
}
void clcd_data_wr(uint8_t data)
{
	clcd_wr(CLCD_DATA, data);
#ifdef CLCD_AUTO_POS
	clcd_pos++;
	if ((clcd_pos == 16) || (clcd_pos == 32))
	{
		clcd_move_to(clcd_pos);
	}
#endif
}

void clcd_init(void) // TODO: Pass Init Control Options
{
#ifdef CLCD_FOUR_BIT
	//LCDDATA |= 0xF0;
	LCDDREG |= 0xF0; // PD4-7 as output
#else
	//LCDDATA = 0xFF;
	LCDDREG = 0xFF; // PD0-7 as output
#endif
	//LCDCTRL &= ~(CLCD_RS | CLCD_EN);
	LCDCREG |= (CLCD_RS | CLCD_EN);

	/*
	 * The following 2 commands are typically not in the CLCD datasheet, but
	 * apparently are HD44780 standard
	 */
	clcd_cmd_wr(0x33);
	clcd_cmd_wr(0x32);
#ifdef CLCD_FOUR_BIT
	clcd_cmd_wr(0x28); /* 001 - 4/8bits - 1/2lines - 5x7/5x10 - ** */
#else
	clcd_cmd_wr(0x38); /* 001 - 4/8bits - 1/2lines - 5x7/5x10 - ** */
#endif
	//clcd_cmd_wr(0x14); /* 0001 - moveCursor/Display - Left/Right - ** */
	clcd_cmd_wr(0x0C); /* 00001 - DissplayOff/On - CursorOff/On - CursorBlinkOff/On */
	clcd_cmd_wr(0x06); /* 000001 - Decr/IncrCursorPos - ShiftDisplayOff/On */
	clcd_cls();
}

void clcd_print_string(char *str)
{
	register uint8_t i = 0;

	while (str[i])
	{
		clcd_data_wr(str[i]);
		i++;
	}
}
