/*
 * Copyright (C) eSrijan Innovations Private Limited
 * 
 * Author: Anil Kumar Pugalia <anil_pugalia@eSrijan.com>
 *
 * Licensed under: JSL (See LICENSE file for details)
 *
 * ATmega48/88/168, ATmega16/32
 *
 * Header for Common Functions for ATmega48/88/168, ATmega16/32
 */

#ifndef COMMON_H
#define COMMON_H

#include <avr/io.h>

void delay_us(uint16_t usecs);
void delay_ms(uint16_t msecs);
#endif
