/*
 * Copyright (C) eSrijan Innovations Private Limited
 * 
 * Author: Anil Kumar Pugalia <anil_pugalia@eSrijan.com>
 *
 * Licensed under: JSL (See LICENSE file for details)
 *
 * ATmega16/32
 *
 * Header for SPI Programmer Functions for ATmega16/32 to program any AVR uC
 */
#ifndef SERIAL_PROGRAMMER_H
#define SERIAL_PROGRAMMER_H

#include <avr/io.h>
#include <avr/interrupt.h>

#define SPM_PS_WORDS (SPM_PAGESIZE >> 1)
#define SPM_SZ_WORDS ((FLASHEND + 1) >> 1)
#define EEP_SZ ((E2END + 1) >> 1)

void init_serial_pgmer(void);
void shut_serial_pgmer(void);

void setup_serial_programming(void);
void reset_serial_programming(void);

void read_sign(uint8_t *msb, uint8_t *csb, uint8_t *lsb);
void read_lock(uint8_t *lock);
void read_fuse(uint8_t *efuse, uint8_t *hfuse, uint8_t *lfuse);
void read_cali(uint8_t *cali);
void write_lock(uint8_t lock);
void write_fuse(uint8_t efuse, uint16_t fuse_word);

void erase_chip(void);
uint8_t erase_ok(void);

uint8_t read_eeprom(uint16_t addr);
void write_eeprom(uint16_t addr, uint8_t byte);
void load_eep_mem_byte(uint16_t addr, uint8_t byte);
void write_eep_mem_page(uint16_t addr);

uint16_t read_pgm_mem_word(uint16_t waddr);
void load_pgm_mem_word(uint16_t waddr, uint16_t word);
void write_pgm_mem_page(uint16_t waddr);
void load_ext_addr(uint8_t ext_addr);

#endif
