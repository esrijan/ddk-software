/*
 * Copyright (C) eSrijan Innovations Private Limited
 * 
 * Author: Anil Kumar Pugalia <anil_pugalia@eSrijan.com>
 *
 * Licensed under: JSL (See LICENSE file for details)
 *
 * ATmega16/32
 *
 * Header for Flash Operation Functions
 */

#ifndef FLASH_OPS_H
#define FLASH_OPS_H

#include <avr/io.h>

uint16_t flash_read_word(uint16_t *addr);
void flash_write_word(uint16_t *addr, uint16_t word);
#endif
