/*
 * Copyright (C) eSrijan Innovations Private Limited
 * 
 * Author: Anil Kumar Pugalia <anil_pugalia@eSrijan.com>
 *
 * Licensed under: JSL (See LICENSE file for details)
 *
 * ATmega16/32
 *
 * Header for Flash Write Block Function
 */

#ifndef FWB_H
#define FWB_H

#include <avr/io.h>

#define BLOCK_SIZE SPM_PAGESIZE /* in bytes */

int flash_write_block(uint8_t *block_addr, uint8_t *data) __attribute__((section(".fwb"), used));
#endif
