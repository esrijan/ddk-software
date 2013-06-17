/*
 * Copyright (C) eSrijan Innovations Private Limited
 * 
 * Author: Anil Kumar Pugalia <anil_pugalia@eSrijan.com>
 *
 * Licensed under: JSL (See LICENSE file for details)
 *
 * ATmega16/32
 * 
 * AVR Parallel Programmer using RESET, XTAL1, RDY/BSYn, OEn, WRn, BS1, XA0,
 * XA1, PAGEL, BS2, DATA (PB7-0) lines of target AVR uC
 * Assuming the following ATmega16/32 to AVR uC connections:
 *	. PC0 -> XTAL1
 *	. PC1 <- RDY/BSYb
 *	. PC2 -> OEb
 *	. PC3 -> WRb
 *	. PC4 -> BS1
 *	. PC5 -> XA0
 *	. PC6 -> XA1
 *	. PC7 -> PAGEL
 *	. PB5 -> BS2
 *	. PB6 -> RESETb
 *	. PA <-> DATA
 * And a ~10K resistor between RESETb & +12V supply // TODO: This technique is
 * not working. Consequently, all code with RESETb becomes TODO
 */

#include <avr/io.h>
#include <util/delay.h>

#include "ddk.h"
#include "parallel_programmer.h"
#ifdef USE_CLCD
#include "clcd.h"
#endif

#define PGMW1 PORTC
#define PGMW2 PORTB
#define PGMR1 PINC
#define PGMR2 PINB
#define DATA_CTL DDRA
#define DATA_OUT PORTA
#define DATA_IN PINA
#define XTAL1 (1 << PC0)
#define RDY_BSYb (1 << PC1)
#define OEb (1 << PC2)
#define WRb (1 << PC3)
#define BS1 (1 << PC4)
#define XA0 (1 << PC5)
#define XA1 (1 << PC6)
#define PAGEL (1 << PC7)
#define BS2 (1 << PB5) /* Exposed at Pin 4 of PGM-DB9 */
#define RESETb (1 << PB6) /* Exposed at Pin 8 of PGM-DB9 */

enum avr_parallel_programming_action
{
	appa_load_addr = 0x0,
	appa_load_data = 0x1,
	appa_load_cmd = 0x2,
	appa_no_action = 0x3
};

#define LOW 0
#define HIGH 1

enum avr_parallel_programming_command
{
	appc_chip_erase = 0x80,
	appc_write_fuse_bits = 0x40,
	appc_write_lock_bits = 0x20,
	appc_write_flash = 0x10,
	appc_write_eeprom = 0x11,
	appc_read_sig_bytes_n_cali_byte = 0x08,
	appc_read_fuse_n_lock_bits = 0x04,
	appc_read_flash = 0x02,
	appc_read_eeprom = 0x03,
	appc_no_operation = 0x00
};

void init_parallel_pgmer(void)
{
#ifdef USE_CLCD
	clcd_init();
	clcd_print_string("DDK PProgrammer");
#endif

	// 1 = output, 0 = input
	// PC0 -> XTAL1, PC2 -> OEb, PC3 -> WRb, PC4 -> BS1, PC5 -> XA0, PC6 -> XA1,
	// PC7 -> PAGEL, PB5 -> BS2, PB6 -> RESETb = output;
	// PC1 <- RDY/BSYb = input; PA <-> DATA = both

	DDRC |= (XTAL1 | OEb | WRb | BS1 | XA0 | XA1 | PAGEL);
	BUTTON_INIT; // TODO
	DDRB |= (BS2 | RESETb);
	DDRC &= ~RDY_BSYb;
	// DDRA aka DATA_CTL will be controlled as needed
}
void shut_parallel_pgmer(void)
{
	DDRC &= ~(XTAL1 | OEb | WRb | BS1 | XA0 | XA1 | PAGEL);
	DDRB &= ~BS2; // RESETb should remain as output. Otherwise, +12V would be applied
}

static void reset(void)
{
	PGMW2 |= RESETb;
	_delay_us(2); // Worst-case 2 CPU ticks (assuming min. of 1MHz clock)
	PGMW2 &= ~RESETb;
}
static void power_up_seq(void)
{
	PGMW1 &= ~(XTAL1 | BS1 | XA0 | XA1 | PAGEL);
	PGMW1 |= (OEb | WRb);
	PGMW2 &= ~(BS2 | RESETb); // TODO: Or shall reset be pulled high to start w/?
	reset(); // TODO: Or shall this be not done?
}
static void execute(void)
{
	PGMW2 &= ~RESETb;
	_delay_us(2); // Worst-case 2 CPU ticks (assuming min. of 1MHz clock)
	PGMW2 |= RESETb;
}
void apply_high_voltage_to_reset(void) // TODO: This technique is not working
{
	// Set RESETb as I/P to allow +12V pass through
	DDRB &= ~RESETb;
}
void remove_high_voltage_from_reset(void) // TODO: This technique is not working
{
	DDRB |= RESETb;
}

static void set_action(uint8_t action)
{
	if (action & 0x2)
	{
		PGMW1 |= XA1;
	}
	else
	{
		PGMW1 &= ~XA1;
	}
	if (action & 0x1)
	{
		PGMW1 |= XA0;
	}
	else
	{
		PGMW1 &= ~XA0;
	}
}
static void set_byte_low(void)
{
	PGMW1 &= ~BS1;
}
static void set_byte_high(void)
{
	PGMW1 |= BS1;
}
static void reset_bs2(void)
{
	PGMW1 &= ~BS1;
}
static void set_bs2(void)
{
	PGMW1 |= BS1;
}
static void data_out(uint8_t data)
{
	DATA_CTL = 0xFF;
	DATA_OUT = data;
}
static uint8_t data_in(void)
{
	DATA_CTL = 0x00;
	return DATA_IN;
}
static void xtal1_pulse(void)
{
	PGMW1 |= XTAL1;
	_delay_us(1);
	PGMW1 &= ~XTAL1;
	_delay_us(1);
}
static void wrb_pulse(void)
{
	PGMW1 &= ~WRb;
	_delay_us(1);
	PGMW1 |= WRb;
	_delay_us(1);
}
static void pagel_pulse(void)
{
	PGMW1 |= PAGEL;
	_delay_us(1);
	PGMW1 &= ~PAGEL;
	_delay_us(1);
}
static void output_enable(void)
{
	PGMW1 &= ~OEb;
}
static void output_disable(void)
{
	PGMW1 |= OEb;
}
static void load_generic(uint8_t action, uint8_t high, uint8_t data)
{
	set_action(action);
	if (high)
		set_byte_high();
	else
		set_byte_low();
	data_out(data);
	xtal1_pulse();
}
static void load_command(uint8_t cmd) /* Datasheet code: A */
{
	load_generic(appa_load_cmd, LOW, cmd);
}
static void load_address_low_byte(uint8_t addr) /* Datasheet code: B */
{
	load_generic(appa_load_addr, LOW, addr);
}
static void load_address_high_byte(uint8_t addr) /* Datasheet code: G */
{
	load_generic(appa_load_addr, HIGH, addr);
}
static void load_data_low_byte(uint8_t data) /* Datasheet code: C */
{
	load_generic(appa_load_data, LOW, data);
}
static void load_data_high_byte(uint8_t data) /* Datasheet code: D */
{
	load_generic(appa_load_data, HIGH, data);
}
static void latch_data(void) /* Datasheet code: E */
{
	set_byte_high();
	pagel_pulse();
}
static uint8_t poll_busy(void)
{
	return !(PGMR1 & RDY_BSYb);
}
static void program_page(void) /* Datasheet code: H & L */
{
	set_byte_low();
	wrb_pulse();
	while (poll_busy())
		;
}

// TODO
#define TS (1 << PB2) // Test Switch bit mask
#define TS_PRESSED (BUTTON_PRESSED) // Test Switch Pressed
static void read_switch() // This also is useful only w/ CLCD
{
	while (!TS_PRESSED)
		;
	_delay_ms(20); // Maximum debouncing period
	while (TS_PRESSED)
		;
	_delay_ms(20); // Maximum debouncing period
}
static void enable_parallel_programming(void) // TODO
{
#if 1
	PGMW1 &= ~(PAGEL | XA1 | XA0 | BS1);
	read_switch();
	// Applied +5V & +12V
	erase_chip();
	write_fuse(0, 0x99E0);
#else
	_delay_us(100);
	PGMW2 &= ~RESETb;
	xtal1_pulse();
	xtal1_pulse();
	xtal1_pulse();
	PGMW1 &= ~(PAGEL | XA1 | XA0 | BS1);
	_delay_us(1);
	read_switch();
	apply_high_voltage_to_reset();
	_delay_us(1);
#endif
}
void setup_parallel_programming(void)
{
	power_up_seq();
	enable_parallel_programming();
}
static void disable_parallel_programming(void)
{
	remove_high_voltage_from_reset();
}
void reset_parallel_programming(void)
{
	disable_parallel_programming();
	execute();
}

void read_sign(uint8_t *msb, uint8_t *csb, uint8_t *lsb)
{
	load_command(appc_read_sig_bytes_n_cali_byte);
	//load_address_high_byte(0); // TODO

	load_address_low_byte(0);
	output_enable();
	set_byte_low();
	*msb = data_in();
	output_disable();

	load_address_low_byte(1);
	output_enable();
	set_byte_low();
	*csb = data_in();
	output_disable();

	load_address_low_byte(2);
	output_enable();
	set_byte_low();
	*lsb = data_in();
	output_disable();
}
void read_lock(uint8_t *lock)
{
	load_command(appc_read_fuse_n_lock_bits);
	output_enable();
	reset_bs2();
	set_byte_high();
	*lock = data_in();
	output_disable();
}
void read_fuse(uint8_t *efuse, uint8_t *hfuse, uint8_t *lfuse)
{
	load_command(appc_read_fuse_n_lock_bits);
	output_enable();
	reset_bs2();
	set_byte_low();
	*lfuse = data_in();
	set_bs2();
	set_byte_high();
	*hfuse = data_in();
	set_bs2();
	set_byte_low();
	*efuse = data_in();
	output_disable();
}
void read_cali(uint8_t *cali)
{
	load_command(appc_read_sig_bytes_n_cali_byte);

	load_address_low_byte(0);
	output_enable();
	set_byte_high();
	*cali = data_in();
	output_disable();
}
void write_lock(uint8_t lock)
{
	load_command(appc_write_lock_bits);
	load_data_low_byte(lock);
	set_byte_low();
	reset_bs2();
	wrb_pulse();
	while (poll_busy())
		;
}
void write_fuse(uint8_t efuse, uint16_t fuse_word)
{
	load_command(appc_write_fuse_bits);

	load_data_low_byte(fuse_word & 0xFF);
	set_byte_low();
	reset_bs2();
	wrb_pulse();
	while (poll_busy())
		;

	load_data_low_byte(fuse_word >> 8);
	set_byte_high();
	reset_bs2();
	wrb_pulse();
	while (poll_busy())
		;
	set_byte_low();

	load_data_low_byte(efuse);
	set_byte_low();
	set_bs2();
	wrb_pulse();
	while (poll_busy())
		;
	reset_bs2();
}

void erase_chip(void)
{
	load_command(appc_chip_erase);
	wrb_pulse();
	while (poll_busy())
		;
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
	uint8_t data;

	load_command(appc_read_eeprom);
	load_address_high_byte(addr >> 8);
	load_address_low_byte(addr & 0xFF);
	output_enable();
	set_byte_low();
	data = data_in();
	output_disable();

	return data;
}
uint16_t read_pgm_mem_word(uint16_t waddr)
{
	uint16_t word;

	load_command(appc_read_flash);
	load_address_high_byte(waddr >> 8);
	load_address_low_byte(waddr & 0xFF);
	output_enable();
	set_byte_low();
	word = data_in();
	set_byte_high();
	word |= data_in() << 8;
	output_disable();

	return word;
}

void initiate_eep_mem_write(void)
{
	load_command(appc_write_eeprom);
}
void load_eep_mem_byte(uint16_t addr, uint8_t byte)
{
	load_address_low_byte(addr & 0xFF);
	load_data_low_byte(byte);
	latch_data();
}
void write_eep_mem_page(uint16_t addr)
{
	load_address_high_byte(addr >> 8);
	program_page();
}
void initiate_pgm_mem_write(void)
{
	load_command(appc_write_flash);
}
void load_pgm_mem_word(uint16_t waddr, uint16_t word)
{
	load_address_low_byte(waddr & 0xFF);
	load_data_low_byte(word & 0xFF);
	load_data_high_byte(word >> 8);
	latch_data();
}
void write_pgm_mem_page(uint16_t waddr)
{
	load_address_high_byte(waddr >> 8);
	program_page();
}
void shutdown_pgm_mem_write(void)
{
	load_command(appc_no_operation);
}
