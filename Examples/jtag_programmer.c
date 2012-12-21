/*
 * Copyright (C) eSrijan Innovations Private Limited
 * 
 * Author: Anil Kumar Pugalia <anil_pugalia@eSrijan.com>
 *
 * Licensed under: JSL (See LICENSE file for details)
 *
 * ATmega16/32
 * 
 * AVR JTAG Programmer using RESET, TCK, TMS, TDO, TDI lines of target AVR uC
 * Assuming the following ATmega16/32 to AVR uC connections:
 *	. PA0 -> RESET
 *	. PA1 -> TCK
 *	. PA2 -> TMS
 *	. PA3 <- TDO
 *	. PA4 -> TDI
 */

#include <avr/io.h>
#include <util/delay.h>

#include "jtag_programmer.h"
#ifdef USE_CLCD
#include "clcd.h"
#endif

#define PGMW PORTA
#define PGMR PINA
#define RESETb (1 << PA0)
#define TCK (1 << PA1)
#define TMS (1 << PA2)
#define TDO (1 << PA3)
#define TDI (1 << PA4)

enum avr_jtag_instr_opcode
{
	aji_extest = 0x0,
	aji_idcode = 0x1,
	aji_sample_preload = 0x2,

	aji_prog_enable = 0x4,
	aji_prog_commands = 0x5, 
	aji_prog_pageload = 0x6,
	aji_prog_pageread = 0x7,
	aji_pvt0 = 0x8,
	aji_pvt1 = 0x9,
	aji_pvt2 = 0xA,
	aji_pvt3 = 0xB,
	aji_avr_reset = 0xC,

	aji_bypass = 0xF
};

#define ENABLE 1
#define DISABLE 0
#define PROG_ENABLE_CODE 0xA370
#define PROG_DISABLE_CODE 0x0000

/* AVR JTAG PGM CMDs */

#define AJP_OPERATION_COMPLETE 0x0200

#define AJP_CHIP_ERASE 0x2380, 0x3180, 0x3380, 0x3380
#define AJP_CHIP_ERASE_POLL 0x3380

#define AJP_FLASH_WRITE_ENTER 0x2310
#define AJP_LOAD_ADDR_HBYTE(h) (0x0700 | ((h) & 0xFF))
#define AJP_LOAD_ADDR_LBYTE(l) (0x0300 | ((l) & 0xFF))
#define AJP_LOAD_DATA_LBYTE(l) (0x1300 | ((l) & 0xFF))
#define AJP_LOAD_DATA_HBYTE(h) (0x1700 | ((h) & 0xFF))
#define AJP_LATCH_DATA 0x3700, 0x7700, 0x3700
#define AJP_FLASH_WRITE_PAGE 0x3700, 0x3500, 0x3700, 0x3700
#define AJP_FLASH_WRITE_PAGE_POLL 0x3700

#define AJP_FLASH_READ_ENTER 0x2302
#define AJP_READ_DATA_LBYTE 0x3200, 0x3600
#define AJP_READ_DATA_HBYTE 0x3700

#define AJP_EEP_WRITE_ENTER 0x2311
#define AJP_LOAD_DATA_BYTE(b) (0x1300 | ((b) & 0xFF))
#define AJP_EEP_WRITE_PAGE 0x3300, 0x3100, 0x3300, 0x3300
#define AJP_EEP_WRITE_PAGE_POLL 0x3300

#define AJP_EEP_READ_ENTER 0x2303
#define AJP_READ_DATA_BYTE 0x3300, 0x3200, 0x3300

#define AJP_FUSE_WRITE_ENTER 0x2340
#define AJP_FUSE_WRITE_HBYTE 0x3700, 0x3500, 0x3700, 0x3700
#define AJP_FUSE_WRITE_HBYTE_POLL 0x3700
#define AJP_FUSE_WRITE_LBYTE 0x3300, 0x3100, 0x3300, 0x3300
#define AJP_FUSE_WRITE_LBYTE_POLL 0x3300

#define AJP_LOCK_BITS_WRITE_ENTER 0x2320
#define AJP_LOAD_DATA_BITS(b) (0x13C0 | ((b) & 0x3F))
#define AJP_LOCK_BITS_WRITE 0x3300, 0x3100, 0x3300, 0x3300
#define AJP_LOCK_BITS_WRITE_POLL 0x3300

#define AJP_FUSE_LOCK_BITS_READ_ENTER 0x2304
#define AJP_FUSE_READ_HBYTE 0x3E00, 0x3F00
#define AJP_FUSE_READ_LBYTE 0x3200, 0x3300
#define AJP_LOCK_BITS_READ 0x3600, 0x3700
#define AJP_FUSE_LOCK_BITS_READ 0x3E00, 0x3200, 0x3600, 0x3700

#define AJP_SIG_BYTE_READ_ENTER 0x2308
#define AJP_LOAD_ADDR_BYTE(a) (0x0300 | ((a) & 0xFF))
#define AJP_SIG_BYTE_READ 0x3200, 0x3300
#define AJP_CALI_BYTE_READ_ENTER 0x2308
#define AJP_CALI_BYTE_READ 0x3600, 0x3700

#define AJP_LOAD_NOP_CMD 0x2300, 0x3300

void init_jtag_pgmer(void)
{
#ifdef USE_CLCD
	clcd_init();
	clcd_print_string("DDK JProgrammer");
#endif

	// 1 = output, 0 = input
	// PA0 -> RESET(bar), PA1 -> TCK, PA2 -> TMS, PA4 -> TDI = output; PA3 <- TDO = input
	DDRA |= (RESETb | TCK | TMS | TDI);
	DDRA &= ~TDO;
}
void shut_jtag_pgmer(void)
{
	DDRA &= ~(RESETb | TCK | TMS | TDI);
}

static void reset(void)
{
	PGMW |= RESETb;
	_delay_us(2); // Worst-case 2 CPU ticks (assuming min. of 1MHz clock)
	PGMW &= ~RESETb;
}
static void power_up_seq(void)
{
	PGMW |= TMS;
	PGMW &= ~(RESETb | TCK);
	reset();
}
static void execute(void)
{
	PGMW |= RESETb;
}
static void clock(void)
{
	PGMW &= ~TCK;
	_delay_us(1);
	PGMW |= TCK;
	_delay_us(1);
}
static void tms_one(void)
{
	PGMW |= TMS;
	clock();
}
static void tms_zero(void)
{
	PGMW &= ~TMS;
	clock();
}
static void tdi_one(void)
{
	PGMW |= TDI;
	clock();
}
static void tdi_zero(void)
{
	PGMW &= ~TDI;
	clock();
}

static void run_test_idle_state(void)
{
	tms_one();
	tms_one();
	tms_one();
	tms_one();
	tms_one();
	tms_zero();
}
// Assuming start state of Run-Test/Idle or eq (i.e. Update-IR or Update-DR) &
// putting into the Update-IR state
static uint8_t _load_instruction(uint8_t instr)
{
	uint8_t prev_cmd_status = 0;

	tms_one();
	tms_one();
	tms_zero();
	tms_zero();

	(instr & 0x1) ? tdi_one() : tdi_zero();
	if (PGMR & TDO) prev_cmd_status |= 0x1;
	(instr & 0x2) ? tdi_one() : tdi_zero();
	if (PGMR & TDO) prev_cmd_status |= 0x2;
	(instr & 0x4) ? tdi_one() : tdi_zero();
	if (PGMR & TDO) prev_cmd_status |= 0x4;
	PGMW |= TMS; // will cause tms_one()
	(instr & 0x8) ? tdi_one() : tdi_zero();
	if (PGMR & TDO) prev_cmd_status |= 0x8;

	tms_one();

	return prev_cmd_status;
}
// Assuming start state of Run-Test/Idle or eq (i.e. Update-IR or Update-DR) &
// putting back into the Run-Test/Idle state
static uint8_t load_instruction(uint8_t instr)
{
	uint8_t prev_cmd_status;

	prev_cmd_status = _load_instruction(instr);

	tms_zero();

	return prev_cmd_status;
}
// Assuming start state of Run-Test/Idle or eq (i.e. Update-IR or Update-DR) &
// putting into the Update-DR state
static uint16_t _load_data(uint16_t data, uint8_t size)
{
	uint16_t read_data = 0;
	uint8_t i;

	tms_one();
	tms_zero();
	tms_zero();

	for (i = 0; i < size - 1; i++)
	{
		(data & (1 << i)) ? tdi_one() : tdi_zero();
		if (PGMR & TDO) read_data |= (1 << i);
	}
	PGMW |= TMS; // will cause tms_one()
	(data & (1 << i)) ? tdi_one() : tdi_zero();
	if (PGMR & TDO) read_data |= (1 << i);

	tms_one();

	return read_data;
}
// Assuming start state of Run-Test/Idle or eq (i.e. Update-IR or Update-DR) &
// putting back into the Run-Test/Idle state
static uint16_t load_data(uint16_t data, uint8_t size)
{
	uint16_t read_data;

	read_data = _load_data(data, size);

	tms_zero();

	return read_data;
}
// Assuming start state of Run-Test/Idle or eq (i.e. Update-IR or Update-DR) &
// putting into Update-DR state
static uint16_t _load_instruction_n_data(uint8_t instr, uint16_t data, uint8_t size)
{
	_load_instruction(instr);
	return _load_data(data, size);
}
// Assuming start state of Run-Test/Idle or eq (i.e. Update-IR or Update-DR) &
// putting back into the Run-Test/Idle state
static uint16_t load_instruction_n_data(uint8_t instr, uint16_t data, uint8_t size)
{
	_load_instruction(instr);
	return load_data(data, size);
}

static uint16_t program_cmd(uint16_t cmd)
{
	return load_instruction_n_data(aji_prog_commands, cmd, 15);
}
static uint16_t program_cmd2(uint16_t cmd1, uint16_t cmd2)
{
	load_instruction_n_data(aji_prog_commands, cmd1, 15);
	return load_data(cmd2, 15);
}
static uint16_t program_cmd3(uint16_t cmd1, uint16_t cmd2, uint16_t cmd3)
{
	load_instruction_n_data(aji_prog_commands, cmd1, 15);
	_load_data(cmd2, 15);
	return load_data(cmd3, 15);
}
static uint16_t program_cmd4(uint16_t cmd1, uint16_t cmd2, uint16_t cmd3, uint16_t cmd4)
{
	load_instruction_n_data(aji_prog_commands, cmd1, 15);
	_load_data(cmd2, 15);
	_load_data(cmd3, 15);
	return load_data(cmd4, 15);
}
static uint16_t load_cmd(uint16_t cmd)
{
	return load_data(cmd, 15);
}
static uint16_t load_cmd2(uint16_t cmd1, uint16_t cmd2)
{
	_load_data(cmd1, 15);
	return load_data(cmd2, 15);
}
static uint16_t load_cmd3(uint16_t cmd1, uint16_t cmd2, uint16_t cmd3)
{
	_load_data(cmd1, 15);
	_load_data(cmd2, 15);
	return load_data(cmd3, 15);
}
static uint16_t load_cmd4(uint16_t cmd1, uint16_t cmd2, uint16_t cmd3, uint16_t cmd4)
{
	_load_data(cmd1, 15);
	_load_data(cmd2, 15);
	_load_data(cmd3, 15);
	return load_data(cmd4, 15);
}
static uint8_t poll_busy(uint16_t cmd)
{
	return !(load_cmd(cmd) & AJP_OPERATION_COMPLETE);
}

static void _avr_reset(uint8_t enable)
{
	_load_instruction_n_data(aji_avr_reset, enable, 1);
}
static void enable_jtag_programming(void)
{
	_avr_reset(ENABLE);
	load_instruction_n_data(aji_prog_enable, PROG_ENABLE_CODE, 16);
}
void setup_jtag_programming(void)
{
	power_up_seq();
	run_test_idle_state();
	enable_jtag_programming();
}
static void disable_jtag_programming(void)
{
	program_cmd2(AJP_LOAD_NOP_CMD);
	load_instruction_n_data(aji_prog_enable, PROG_DISABLE_CODE, 16);
	_avr_reset(DISABLE);
	tms_zero();
}
void reset_jtag_programming(void)
{
	disable_jtag_programming();
	run_test_idle_state();
	execute();
}

void read_sign(uint8_t *msb, uint8_t *csb, uint8_t *lsb)
{
	program_cmd(AJP_SIG_BYTE_READ_ENTER);

	load_cmd(AJP_LOAD_ADDR_BYTE(0));
	*msb = load_cmd2(AJP_SIG_BYTE_READ);

	load_cmd(AJP_LOAD_ADDR_BYTE(1));
	*csb = load_cmd2(AJP_SIG_BYTE_READ);

	load_cmd(AJP_LOAD_ADDR_BYTE(2));
	*lsb = load_cmd2(AJP_SIG_BYTE_READ);
}
void read_lock(uint8_t *lock)
{
	program_cmd(AJP_FUSE_LOCK_BITS_READ_ENTER);
	*lock = load_cmd2(AJP_LOCK_BITS_READ);
}
void read_fuse(uint8_t *efuse, uint8_t *hfuse, uint8_t *lfuse)
{
	program_cmd(AJP_FUSE_LOCK_BITS_READ_ENTER);
	*efuse = 0x00; // Typically no EFUSE for JTAG supported AVRs
	*hfuse = load_cmd2(AJP_FUSE_READ_HBYTE);
	*lfuse = load_cmd2(AJP_FUSE_READ_LBYTE);
}
void read_cali(uint8_t *cali)
{
	program_cmd(AJP_CALI_BYTE_READ_ENTER);
	load_cmd(AJP_LOAD_ADDR_BYTE(0));
	*cali = load_cmd2(AJP_CALI_BYTE_READ);
}
void write_lock(uint8_t lock)
{
	program_cmd(AJP_LOCK_BITS_WRITE_ENTER);
	load_cmd(AJP_LOAD_DATA_BITS(lock));
	load_cmd4(AJP_LOCK_BITS_WRITE);
	while (poll_busy(AJP_LOCK_BITS_WRITE_POLL))
		;
}
void write_fuse(uint8_t efuse, uint16_t fuse_word)
{
	program_cmd(AJP_FUSE_WRITE_ENTER);

	(void)(efuse); // Typically no EFUSE for JTAG supported AVRs

	load_cmd(AJP_LOAD_DATA_LBYTE(fuse_word >> 8));
	load_cmd4(AJP_FUSE_WRITE_HBYTE);
	while (poll_busy(AJP_FUSE_WRITE_HBYTE_POLL))
		;

	load_cmd(AJP_LOAD_DATA_LBYTE(fuse_word & 0xFF));
	load_cmd4(AJP_FUSE_WRITE_LBYTE);
	while (poll_busy(AJP_FUSE_WRITE_LBYTE_POLL))
		;
}

void erase_chip(void)
{
	program_cmd4(AJP_CHIP_ERASE);
	while (poll_busy(AJP_CHIP_ERASE_POLL))
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
	program_cmd(AJP_EEP_READ_ENTER);
	load_cmd2(AJP_LOAD_ADDR_HBYTE(addr >> 8), AJP_LOAD_ADDR_LBYTE(addr & 0xFF));
	return (load_cmd3(AJP_READ_DATA_BYTE) & 0xFF);
}
uint16_t read_pgm_mem_word(uint16_t waddr)
{
	uint16_t word;

	program_cmd(AJP_FLASH_READ_ENTER);
	load_cmd2(AJP_LOAD_ADDR_HBYTE(waddr >> 8), AJP_LOAD_ADDR_LBYTE(waddr & 0xFF));
	word = load_cmd2(AJP_READ_DATA_LBYTE) & 0xFF;
	word |= (load_cmd(AJP_READ_DATA_HBYTE) & 0xFF) << 8;
	return word;
}

void initiate_eep_mem_write(void)
{
	program_cmd(AJP_EEP_WRITE_ENTER);
}
void load_eep_mem_byte(uint16_t addr, uint8_t byte)
{
	load_cmd(AJP_LOAD_ADDR_HBYTE(addr >> 8));
	load_cmd(AJP_LOAD_ADDR_LBYTE(addr & 0xFF));
	load_cmd(AJP_LOAD_DATA_BYTE(byte));
	load_cmd3(AJP_LATCH_DATA);
}
void write_eep_mem_page(uint16_t addr)
{
	(void)(addr); // Not used
	load_cmd4(AJP_EEP_WRITE_PAGE);
	while (poll_busy(AJP_EEP_WRITE_PAGE_POLL))
		;
}
void initiate_pgm_mem_write(void)
{
	program_cmd(AJP_FLASH_WRITE_ENTER);
}
void load_pgm_mem_word(uint16_t waddr, uint16_t word)
{
	load_cmd(AJP_LOAD_ADDR_HBYTE(waddr >> 8));
	load_cmd(AJP_LOAD_ADDR_LBYTE(waddr & 0xFF));
	load_cmd(AJP_LOAD_DATA_LBYTE(word & 0xFF));
	load_cmd(AJP_LOAD_DATA_HBYTE(word >> 8));
	load_cmd3(AJP_LATCH_DATA);
}
void write_pgm_mem_page(uint16_t waddr)
{
	(void)(waddr); // Not used
	load_cmd4(AJP_FLASH_WRITE_PAGE);
	while (poll_busy(AJP_FLASH_WRITE_PAGE_POLL))
		;
}
