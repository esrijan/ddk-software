/*
 * Copyright (C) eSrijan Innovations Private Limited
 * 
 * Author: Anil Kumar Pugalia <anil_pugalia@eSrijan.com>
 *
 * Licensed under: JSL (See LICENSE file for details)
 *
 * ATmega16/32
 * 
 * AVR Serial Programmer using RESET, SCK, MISO, MOSI lines of target AVR uC
 * Assuming the following ATmega16/32 to AVR uC connections:
 *	. PA0 -> RESET
 *	. PA1 -> SCK
 *	. PA2 <- MISO
 *	. PA3 -> MOSI
 */

#include <avr/io.h>
#include <util/delay.h>

#include "serial_programmer.h"
#ifdef USE_CLCD
#include "clcd.h"
#endif

#define PGMW PORTA
#define PGMR PINA
#define RESETb (1 << PA0)
#define SCK (1 << PA1)
#define MISO (1 << PA2)
#define MOSI (1 << PA3)

void init_serial_pgmer(void)
{
#ifdef USE_CLCD
	clcd_init();
	clcd_print_string("DDK SProgrammer");
#endif

	// 1 = output, 0 = input
	// PA0 -> RESET(bar), PA1 -> SCK, PA3 -> MOSI = output; PA2 <- MISO = input
	DDRA |= (RESETb | SCK | MOSI);
	DDRA &= ~MISO;
}
void shut_serial_pgmer(void)
{
	DDRA &= ~(RESETb | SCK | MOSI);
}

static void reset(void)
{
	PGMW |= RESETb;
	_delay_us(2); // Worst-case 2 CPU ticks (assuming min. of 1MHz clock)
	PGMW &= ~RESETb;
}
static void power_up_seq(void)
{
	PGMW &= ~(RESETb | SCK);
	reset();
}
static void execute(void)
{
	PGMW |= RESETb;
}

static void clock(void)
{
	PGMW |= SCK;
	_delay_us(2); // Worst-case 2 CPU ticks (assuming min. of 1MHz clock)
	PGMW &= ~SCK;
}
static uint8_t read_byte(void)
{
	int8_t i;
	uint8_t byte = 0;

	for (i = 7; i >= 0; i--)
	{
		clock();
		if (PGMR & MISO)
		{
			byte |= (1 << i);
		}
	}
	return byte;
}
static uint8_t write_byte(uint8_t byte)
{
	int8_t i;
	uint8_t echo = 0;

	for (i = 7; i >= 0; i--)
	{
		if (byte & (1 << i))
		{
			PGMW |= MOSI;
		}
		else
		{
			PGMW &= ~MOSI;
		}
		clock();
		if (PGMR & MISO)
		{
			echo |= (1 << i);
		}
	}
	return echo;
}
static uint8_t read_instruction(uint8_t b1, uint8_t b2, uint8_t b3)
{
	write_byte(b1);
	write_byte(b2);
	write_byte(b3);
	return read_byte();
}
static void write_instruction(uint8_t b1, uint8_t b2, uint8_t b3, uint8_t b4)
{
	write_byte(b1);
	write_byte(b2);
	write_byte(b3);
	write_byte(b4);
}
static uint8_t poll_busy(void)
{
	return (read_instruction(0xF0, 0x00, 0x00) & 0x01);
}
static void write_n_complete_instruction(uint8_t b1, uint8_t b2, uint8_t b3, uint8_t b4)
{
	write_instruction(b1, b2, b3, b4);
	while (poll_busy())
		;
}

static void enable_serial_programming(void)
{
	uint8_t second;

	do
	{
		write_byte(0xAC);
		write_byte(0x53);
		second = write_byte(0x00);
		write_byte(0x00);
		if (second != 0x53)
		{
			reset();
		}
	} while (second != 0x53);
}
void setup_serial_programming(void)
{
	power_up_seq();
	_delay_ms(20);
	enable_serial_programming();
}
void reset_serial_programming(void)
{
	execute();
}

void read_sign(uint8_t *msb, uint8_t *csb, uint8_t *lsb)
{
	*msb = read_instruction(0x30, 0x00, 0x00);
	*csb = read_instruction(0x30, 0x00, 0x01);
	*lsb = read_instruction(0x30, 0x00, 0x02);
}
void read_lock(uint8_t *lock)
{
	*lock = read_instruction(0x58, 0x00, 0x00);
}
void read_fuse(uint8_t *efuse, uint8_t *hfuse, uint8_t *lfuse)
{
	*efuse = read_instruction(0x50, 0x08, 0x00);
	*hfuse = read_instruction(0x58, 0x08, 0x00);
	*lfuse = read_instruction(0x50, 0x00, 0x00);
}
void read_cali(uint8_t *cali)
{
	*cali = read_instruction(0x38, 0x00, 0x00);
}
void write_lock(uint8_t lock)
{
	write_n_complete_instruction(0xAC, 0xE0, 0x00, lock);
}
void write_fuse(uint8_t efuse, uint16_t fuse_word)
{
	write_n_complete_instruction(0xAC, 0xA0, 0x00, (fuse_word & 0xFF));
	write_n_complete_instruction(0xAC, 0xA8, 0x00, (fuse_word >> 8));
	write_n_complete_instruction(0xAC, 0xA4, 0x00, efuse);
}

void erase_chip(void)
{
	write_n_complete_instruction(0xAC, 0x80, 0x00, 0x00);
}
uint8_t erase_ok(void)
{
	uint16_t waddr, addr;

	for (waddr = 0; waddr < SPM_SZ_WORDS; waddr++)
	{
		if (read_pgm_mem_word(waddr) != 0xFFFF)
		{
			break;
		}
	}
	if (waddr != SPM_SZ_WORDS)
	{
		return 0;
	}
	for (addr = 0; addr < EEP_SZ; addr++)
	{
		if (read_eeprom(addr) != 0xFF)
		{
			break;
		}
	}
	if (addr != EEP_SZ)
	{
		return 0;
	}
	return 1;
}

uint8_t read_eeprom(uint16_t addr)
{
	return read_instruction(0xA0, (addr >> 8), (addr & 0xFF));
}
uint16_t read_pgm_mem_word(uint16_t waddr)
{
	uint16_t word;

	word = read_instruction(0x20, (waddr >> 8), (waddr & 0xFF));
	word |= read_instruction(0x28, (waddr >> 8), (waddr & 0xFF)) << 8;
	return word;
}

void write_eeprom(uint16_t addr, uint8_t byte)
{
	write_n_complete_instruction(0xC0, (addr >> 8), (addr & 0xFF), byte);
}
void load_eep_mem_byte(uint16_t addr, uint8_t byte)
{
	write_n_complete_instruction(0xC1, 0x00, (addr & (E2PAGESIZE - 1)), byte);
}
void write_eep_mem_page(uint16_t addr)
{
	write_n_complete_instruction(0xC2, (addr >> 8), (addr & 0xFF), 0x00);
}
void load_pgm_mem_word(uint16_t waddr, uint16_t word)
{
	write_n_complete_instruction(0x40, 0x00, (waddr & (SPM_PS_WORDS - 1)), (word & 0xFF));
	write_n_complete_instruction(0x48, 0x00, (waddr & (SPM_PS_WORDS - 1)), (word >> 8));
}
void write_pgm_mem_page(uint16_t waddr)
{
	write_n_complete_instruction(0x4C, (waddr >> 8), (waddr & 0xFF), 0x00);
}
void load_ext_addr(uint8_t ext_addr)
{
	write_n_complete_instruction(0x4D, 0x00, ext_addr, 0x00);
}
