/*
 * Copyright (C) eSrijan Innovations Private Limited
 * 
 * Author: Anil Kumar Pugalia <anil_pugalia@eSrijan.com>
 *
 * Licensed under: JSL (See LICENSE file for details)
 *
 * ATmega16/32
 * 
 * AVR Parallel Programmer Demo using RESET, XTAL1, RDY/BSYn, OEn, WRn, BS1,
 * XA0, XA1, PAGEL, BS2, DATA (PB7-0) lines of target ATmega16
 * Assuming the following ATmega16/32 to ATmega16 connections:
 *	. PC0 -> XTAL1
 *	. PC1 <- RDY/BSYb
 *	. PC2 -> OEb
 *	. PC3 -> WRb
 *	. PC4 -> BS1
 *	. PC5 -> XA0
 *	. PC6 -> XA1
 *	. PC7 -> PAGEL
 *	. PB5 -> BS2 (PB5 is exposed at Pin 4 of PGM-DB9 on DDK)
 *	. PB6 -> RESETb (PB6 is exposed at Pin 8 of PGM-DB9 on DDK)
 *	. PA <-> DATA
 * And a ~10K resistor between RESETb & +12V supply // TODO: This technique is not working
 */

#include <avr/io.h>
#include <util/delay.h>

#ifdef USE_CLCD
#include "clcd.h"
#endif
#include "parallel_programmer.h"
#include "code.h"

#define T_PORT DDRB
#define TL (1 << PB7) // Test LED bit mask
#define TS (1 << PB2) // Test Switch bit mask
#define TL_ON (PORTB |= TL) // Test LED On
#define TL_OFF (PORTB &= ~TL) // Test LED Off
#define TL_TOGGLE (PORTB ^= TL) // Test LED Toggle
#define TS_PRESSED (!!(PINB & TS)) // Test Switch Pressed

void init_parallel_demo(void)
{
	// 1 = output, 0 = input
	T_PORT |= TL;
	T_PORT &= ~TS;
}

#ifdef USE_CLCD
void debug_print_string(char *str)
{
	clcd_move_to(16);
	clcd_print_string("                ");
	clcd_move_to(16);
	clcd_print_string(str);
}
static void debug_char(char c)
{
	char a[3];

	a[0] = (c >> 4) & 0x0F;
	a[1] = c & 0x0F;
	a[2] = 0;
	a[0] += (a[0] < 10) ? '0' : ('A' - 10);
	a[1] += (a[1] < 10) ? '0' : ('A' - 10);
	clcd_print_string(a);
}
void display_sign(void)
{
	uint8_t msb, csb, lsb;

	read_sign(&msb, &csb, &lsb);
	debug_print_string("SIGN: 0x");
	debug_char(msb);
	debug_char(csb);
	debug_char(lsb);
}
void display_lock(void)
{
	uint8_t lock;

	read_lock(&lock);
	debug_print_string("LOCK: 0x");
	debug_char(lock);
}
void display_fuse(void)
{
	uint8_t efuse, hfuse, lfuse;

	read_fuse(&efuse, &hfuse, &lfuse);
	debug_print_string("FUSE: 0x");
	debug_char(efuse);
	debug_char(hfuse);
	debug_char(lfuse);
}
void display_cali(void)
{
	uint8_t cali;

	read_cali(&cali);
	debug_print_string("CALI: 0x");
	debug_char(cali);
}

static void read_switch() // This also is useful only w/ CLCD
{
	while (!TS_PRESSED)
		;
	_delay_ms(20); // Maximum debouncing period
	while (TS_PRESSED)
		;
	_delay_ms(20); // Maximum debouncing period
}
void debug_print_wait(char *str)
{
	debug_print_string(str);
	read_switch();
}
#else
void debug_print_string(char *str)
{
	(void)(str);
}
void debug_print_wait(char *str)
{
	(void)(str);
}
#endif

void pgm_mem(void)
{
	uint16_t waddr;

	initiate_pgm_mem_write();
	for (waddr = 0; waddr < CODE_SZ_WORDS; waddr++)
	{
		load_pgm_mem_word(waddr, code[waddr]);
		if ((waddr & (SPM_PS_WORDS - 1)) == (SPM_PS_WORDS - 1)) // Page's last word
		{
			write_pgm_mem_page(waddr);
		}
	}
	for (; waddr < (CODE_SZ_WORDS + SPM_PS_WORDS - 1) / SPM_PS_WORDS * SPM_PS_WORDS; waddr++)
	{
		load_pgm_mem_word(waddr, 0xFFFF);
		if ((waddr & (SPM_PS_WORDS - 1)) == (SPM_PS_WORDS - 1)) // Page's last word
		{
			write_pgm_mem_page(waddr);
		}
	}
	shutdown_pgm_mem_write();
}
uint8_t pgm_ok(void)
{
	uint16_t waddr;

	for (waddr = 0; waddr < CODE_SZ_WORDS; waddr++)
	{
		if (read_pgm_mem_word(waddr) != code[waddr])
		{
			break;
		}
	}
	return (waddr == CODE_SZ_WORDS) ? 1 : 0;
}

int main(void)
{
	int ts_pressed = 1;

	init_parallel_pgmer();
	init_parallel_demo();

#ifdef USE_CLCD // Displaying various uC parameters make sense only w/ CLCD
	setup_parallel_programming();
	display_sign(); TL_TOGGLE; read_switch();
	display_lock(); TL_TOGGLE; read_switch();
	display_fuse(); TL_TOGGLE; read_switch();
	display_cali(); TL_TOGGLE; read_switch();
	reset_parallel_programming();
#endif

	while (1)
	{
		if (ts_pressed && !TS_PRESSED)
		// Let the code execute in the programmed uC
		{
			TL_OFF;
			debug_print_string("Executing");
			ts_pressed = 0;
		}
		else if (!ts_pressed && TS_PRESSED)
		// Let's program the uC
		{
			TL_ON;
			debug_print_string("JTAG Pgming");
			setup_parallel_programming();
			debug_print_string("Erasing Chip");
			erase_chip();
			if (erase_ok())
			{
				debug_print_wait("Erase: Okay");
			}
			else
			{
				debug_print_wait("Erase: Fail");
			}
			debug_print_string("Pgming Memory");
			pgm_mem();
			if (pgm_ok())
			{
				debug_print_wait("Program: Okay");
			}
			else
			{
				debug_print_wait("Program: Fail");
			}
			//write_fuse(0, 0x99E1); // Default for ATmega16/32. Be doubly sure b4 uncommenting this
			//write_fuse(0x1, 0xDFE2); // Default modified for ATmega168. Be doubly sure b4 uncommenting this
			//write_lock(0x0F);
#ifdef USE_CLCD // Displaying make sense only w/ CLCD
			display_fuse();
			read_switch();
			display_lock();
			read_switch();
#endif
			reset_parallel_programming();
			ts_pressed = 1;
		}
		_delay_ms(1);
	}

	shut_parallel_pgmer();

	return 0;
}
