/*
 * Copyright (C) eSrijan Innovations Private Limited
 * 
 * Author: Anil Kumar Pugalia <anil_pugalia@eSrijan.com>
 *
 * Licensed under: JSL (See LICENSE file for details)
 *
 * ATmega16/32
 *
 * Header for Flash Operation Functions for DDK v2.1
 */

#ifndef FLASH_H
#define FLASH_H

#include <avr/io.h>

#define BLOCK_SIZE SPM_PAGESIZE /* in bytes */

uint8_t flash_read_byte(const uint8_t *addr);
/* block_addr should be BLOCK_SIZE aligned */
int flash_read_block(const uint8_t *block_addr, uint8_t *data);
int (*flash_write_block)(uint8_t *block_addr, uint8_t *data);
#endif
