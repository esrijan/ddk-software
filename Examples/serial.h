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
 * NB Correct operation of USART module is expected only through a MAX232, i.e.
 * inverted, for both an original serial or USB2Serial (0-5V)
 */

#ifndef SERIAL_H
#define SERIAL_H

#include <avr/io.h>

void usart_init(unsigned long baud);
void usart_byte_tx(uint8_t data);
int usart_byte_available(void);
uint8_t usart_byte_rx(void);
void usart_tx(char *str);
void usart_rx(char *str, int max_len);
#endif
