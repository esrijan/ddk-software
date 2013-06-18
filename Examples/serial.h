/*
 * Copyright (C) eSrijan Innovations Private Limited
 * 
 * Author: Anil Kumar Pugalia <anil_pugalia@eSrijan.com>
 *
 * Licensed under: JSL (See LICENSE file for details)
 *
 * ATmega16/32
 *
 * Header for Serial Communication Functions
 * NB Correct operation of USART module w/ PC is expected only through a MAX232,
 * i.e. inverted, for both an original serial or USB2Serial (0-5V)
 *
 * For using this library on DDK's, make sure of the following:
 * 	DDK v1.1: RX jumper should be on the left-most pin pair.
 * 	DDK v2.1: Both RX-TX jumpers should be on the right-most pin pairs.
 */

#ifndef SERIAL_H
#define SERIAL_H

#include <avr/io.h>

/*
 * Supported baud rates: (accuracy may vary based on the clock rates)
 * + For all clocks (>= 1MHz): 2400, 4800, 9600, 14.4K, 19.2K, 28.8K, 38.4K, 57.6K
 * + For clocks >= 1.8432 MHz: 76.8K, 115.2K
 * + For clocks >= 3.6864 MHz: 230.4K, 250K
 * Supported data bits: 5, 6, 7, 8, 9
 * Supported parity: none (N), even (E), odd (O)
 * Supported stop bits: 1, 2
 * Currently, hardcoded setting is 8N1
 */

typedef enum
{
	p_none,
	p_even,
	p_odd
} Parity;

void usart_init(unsigned long baud);
void usart_shut(void);
void usart_enable(void);
void usart_disable(void);
void usart_byte_tx(uint8_t data);
int usart_byte_available(void);
uint8_t usart_byte_rx(void);
void usart_tx(char *str);
void usart_rx(char *str, int max_len);
#endif
