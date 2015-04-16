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

#ifdef FWB

#if (FLASHEND) > 0xFFFF /* we need long addressing */
int __attribute__((section(".fwb"), used)) flash_write_block(uint32_t block_addr, uint8_t *data);
#else
int __attribute__((section(".fwb"), used)) flash_write_block(uint16_t block_addr, uint8_t *data);
#endif

#else

#define flash_write_block(block_addr, data)

#endif

#endif
